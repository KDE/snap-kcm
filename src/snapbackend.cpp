/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "snapbackend.h"
#include "kcmplug.h"
#include <KLocalizedString>
#include <QDBusInterface>

using namespace Qt::Literals::StringLiterals;

SnapBackend::SnapBackend()
{
    // Initalize locals
    QList<QSnapdSnap *> loadedSnaps;
    QList<QSnapdPlug *> loadedPlugs;
    QScopedPointer<QSnapdGetSnapsRequest> reqGetSnaps{m_client.getSnaps()};
    QScopedPointer<QSnapdGetConnectionsRequest> reqGetConnections{m_client.getConnections(QSnapdClient::GetConnectionsFlag::SelectAll)};
    QScopedPointer<QSnapdGetInterfaces2Request> reqGetInterfaces{m_client.getInterfaces2(QSnapdClient::IncludeSlots)};

    // Fetch snaps
    if (reqGetSnaps) {
        reqGetSnaps->runSync();
        for (int i = 0; i < reqGetSnaps->snapCount(); ++i) {
            if (reqGetSnaps->snap(i)->appCount() != 0 && reqGetSnaps->snap(i)->confinement() != QSnapdEnums::SnapConfinementClassic) {
                loadedSnaps.append(reqGetSnaps->snap(i));
            }
        }
        std::sort(loadedSnaps.begin(), loadedSnaps.end(), SnapBackend::comparebyName);

        Q_EMIT snapsChanged();
    }
    // Fetch connections
    QStringList hiddenPlugs = {u"x11"_s,
                               u"content"_s,
                               u"cups"_s,
                               u"desktop"_s,
                               u"desktop-legacy"_s,
                               u"mir"_s,
                               u"wayland"_s,
                               u"unity7"_s,
                               u"opengl"_s};

    if (reqGetConnections) {
        reqGetConnections->runSync();
        for (int i = 0; i < reqGetConnections->plugCount(); ++i) {
            loadedPlugs.append(reqGetConnections->plug(i));
        }
    }

    if (reqGetInterfaces) {
        reqGetInterfaces->runSync();
        for (int i = 0; i < reqGetInterfaces->interfaceCount(); ++i) {
            m_interfaces.append(reqGetInterfaces->interface(i));
        }
    }

    // Get snaps and their associated plugs and slots
    for (QSnapdSnap *snap : loadedSnaps) {
        QList<KCMPlug *> plugsForSnap;
        for (QSnapdPlug *plug : loadedPlugs) {
            if (plug->snap() == snap->name() && (!plug->hasAttribute(u"content"_s) && !(hiddenPlugs.contains(plug->name())))) {
                plugsForSnap.append(new KCMPlug(plug, getPlugLabel(plug->interface()), plugIcon(plug->interface())));
            }
        }
        if (!plugsForSnap.isEmpty()) {
            m_snaps.append(new KCMSnap(snap, plugsForSnap));
        }
    }
}

/**
 * Returns a list of KCMSnap objects filtered by the given filter string.
 *
 * If the filter string is empty, all snaps are returned. Otherwise, only
 * snaps whose names contain the filter string, case-insensitively, are
 * included in the returned list.
 *
 * @param filter The filter string to match snap names against.
 * @return A list of filtered KCMSnap objects.
 */
const QList<KCMSnap *> SnapBackend::snaps(const QString &filter) const
{
    QList<KCMSnap *> filteredSnaps;

    if (filter.isEmpty()) {
        return m_snaps;
    }

    for (KCMSnap *snap : m_snaps) {
        if (snap->name().contains(filter, Qt::CaseInsensitive)) {
            filteredSnaps.append(snap);
        }
    }

    return filteredSnaps;
}

// const QList<KCMSnap *> SnapBackend::slotSnaps() const
// {
//     return m_slotSnaps;
// }

/**
 * Connect a plug from plug_snap with name plug_name to a slot from
 * slot_snap with name slot_name. If the connection is successful,
 * returns an empty string. Otherwise returns an error message.
 *
 * @param plug_snap
 * @param plug_name
 * @param slot_snap
 * @param slot_name
 */
QString SnapBackend::connectPlug(const QString &plug_snap, const QString &plug_name, const QString &slot_snap, const QString &slot_name) const
{
    QSnapdClient client;
    QSnapdConnectInterfaceRequest *req = client.connectInterface(plug_snap, plug_name, slot_snap, slot_name);
    req->runSync();
    if (req->error() != QSnapdRequest::NoError) {
        return req->errorString();
    }
    return QString();
}

/**
 * Disconnect a plug from plug_snap with name plug_name from a slot from
 * slot_snap with name slot_name. If the disconnection is successful,
 * returns an empty string. Otherwise returns an error message.
 *
 * @param plug_snap
 * @param plug_name
 * @param slot_snap
 * @param slot_name
 */
QString SnapBackend::disconnectPlug(const QString &plug_snap, const QString &plug_name, const QString &slot_snap, const QString &slot_name) const
{
    QSnapdClient client;
    QSnapdDisconnectInterfaceRequest *req = client.disconnectInterface(plug_snap, plug_name, slot_snap, slot_name);
    req->runSync();
    if (req->error() != QSnapdRequest::NoError) {
        return req->errorString();
    }
    return QString();
}

/**
 * Compares two snaps by name. Returns true if a's name is less than b's name, false otherwise.
 *
 * @param a the first snap to compare
 * @param b the second snap to compare
 */
bool SnapBackend::comparebyName(QSnapdSnap *a, QSnapdSnap *b)
{
    return a->name() < b->name();
}

/**
 * Launches the given snap if it has a desktop file.
 *
 * This is used when the user wants to launch a snap from the kcm.
 *
 * @param snap the snap to launch
 */
void SnapBackend::invokeDesktopApp(QString desktop) const
{
    QDBusInterface interface(u"io.snapcraft.Launcher"_s,
                             u"/io/snapcraft/PrivilegedDesktopLauncher"_s,
                             u"io.snapcraft.PrivilegedDesktopLauncher"_s,
                             QDBusConnection::sessionBus());
    interface.call(u"OpenDesktopEntry"_s, desktop);
}

const QStringList SnapBackend::getSlotSnap(QString interface) const
{
    QStringList slotSnaps;
    for (auto iface : m_interfaces) {
        if (iface->name() == interface) {
            for (int i = 0; i < iface->slotCount(); i++) {
                slotSnaps.append(iface->slot(i)->snap());
            }
        }
    }
    return slotSnaps;
}

const QString SnapBackend::getPlugLabel(const QString interface)
{
    for (auto iface : m_interfaces) {
        if (iface->name() == interface) {
            return SnapBackend::capitalize(iface->summary());
        }
    }
    return QString();
}

/**
 * Returns an icon name for a given plug name.
 *
 * This function returns an icon name from the Breeze icon theme that is
 * associated with the given plug name. If the plug name is not found in the
 * map, it returns a default icon name.
 *
 * @param plugName the name of the plug
 * @return the icon name associated with the plug name
 */
const QString SnapBackend::plugIcon(const QString &plugName)
{
    static const QMap<QString, QString> plugIcons = {{u"account-control"_s, u"user-identity"_s},
                                                     {u"accounts-service"_s, u"user-identity"_s},
                                                     {u"alsa"_s, u"audio-card"_s},
                                                     {u"appstream-metadata"_s, u"package-x-generic"_s},
                                                     {u"avahi-control"_s, u"network-wired"_s},
                                                     {u"avahi-observe"_s, u"network-wired"_s},
                                                     {u"audio-playback"_s, u"audio-headphones"_s},
                                                     {u"audio-record"_s, u"microphone-sensitivity-high"_s},
                                                     {u"avahi-observe"_s, u"network-workgroup"_s},
                                                     {u"bluetooth-control"_s, u"preferences-system-bluetooth"_s},
                                                     {u"bluez"_s, u"preferences-system-bluetooth"_s},
                                                     {u"browser-support"_s, u"security-high"_s},
                                                     {u"camera"_s, u"camera-photo"_s},
                                                     {u"cups"_s, u"printer"_s},
                                                     {u"cups-control"_s, u"printer"_s},
                                                     {u"joystick"_s, u"input-gaming"_s},
                                                     {u"docker"_s, u"docker"_s},
                                                     {u"desktop"_s, u"application-x-desktop"_s},
                                                     {u"desktop-legacy"_s, u"application-x-desktop"_s},
                                                     {u"desktop-launch"_s, u"kt-start"_s},
                                                     {u"firewall-control"_s, u"preferences-system-firewall"_s},
                                                     {u"fuse-support"_s, u"folder-lock"_s},
                                                     {u"fwupd"_s, u"software-update-available"_s},
                                                     {u"gsettings"_s, u"settings-configure"_s},
                                                     {u"hardware-observe"_s, u"preferences-system"_s},
                                                     {u"hardware-random-control"_s, u"preferences-desktop-random"_s},
                                                     {u"hardware-random-observe"_s, u"preferences-desktop-random"_s},
                                                     {u"home"_s, u"user-home"_s},
                                                     {u"libvirt"_s, u"preferences-system-network"_s},
                                                     {u"locale-control"_s, u"preferences-desktop-locale"_s},
                                                     {u"location-control"_s, u"preferences-location"_s},
                                                     {u"location-observe"_s, u"preferences-location"_s},
                                                     {u"log-observe"_s, u"document-edit"_s},
                                                     {u"lxd"_s, u"utilities-terminal"_s},
                                                     {u"lxd-support"_s, u"utilities-terminal"_s},
                                                     {u"modem-manager"_s, u"network-mobile"_s},
                                                     {u"mount-observe"_s, u"drive-harddisk"_s},
                                                     {u"mpris"_s, u"multimedia-player"_s},
                                                     {u"network"_s, u"network-connect"_s},
                                                     {u"network-bind"_s, u"network-manager"_s},
                                                     {u"network-control"_s, u"network-manager"_s},
                                                     {u"network-manager"_s, u"network-manager"_s},
                                                     {u"network-observe"_s, u"network-manager"_s},
                                                     {u"network-manager"_s, u"network-manager"_s},
                                                     {u"network-manager-observe"_s, u"network-manager"_s},
                                                     {u"network-status"_s, u"network-manager"_s},
                                                     {u"ofono"_s, u"modem"_s},
                                                     {u"openvswitch"_s, u"network"_s},
                                                     {u"optical-drive"_s, u"drive-optical"_s},
                                                     {u"packagekit-control"_s, u"kpackagekit-updates"_s},
                                                     {u"password-manager-service"_s, u"dialog-password"_s},
                                                     {u"pcscd"_s, u"secure-card"_s},
                                                     {u"ppp"_s, u"network"_s},
                                                     {u"process-control"_s, u"system-run"_s},
                                                     {u"pulseaudio"_s, u"audio-volume-high"_s},
                                                     {u"raw-usb"_s, u"drive-removable-media"_s},
                                                     {u"removable-media"_s, u"drive-removable-media"_s},
                                                     {u"screen-inhibit-control"_s, u"video-display"_s},
                                                     {u"serial-port"_s, u"port-serial"_s},
                                                     {u"shared-memory"_s, u"memory"_s},
                                                     {u"shutdown"_s, u"system-shutdown"_s},
                                                     {u"snapd-control"_s, u"folder-snap"_s},
                                                     {u"ssh-keys"_s, u"network-server"_s},
                                                     {u"ssh-public-keys"_s, u"network-server"_s},
                                                     {u"storage-framework-service"_s, u"drive-harddisk"_s},
                                                     {u"system-observe"_s, u"utilities-system-monitor"_s},
                                                     {u"system-packages-doc"_s, u"documentation"_s},
                                                     {u"system-trace"_s, u"document-preview"_s},
                                                     {u"time-control"_s, u"preferences-system-time"_s},
                                                     {u"timeserver-control"_s, u"preferences-system-time"_s},
                                                     {u"timezone-control"_s, u"preferences-system-time"_s},
                                                     {u"udisks2"_s, u"drive-harddisk"_s},
                                                     {u"upower-observe"_s, u"battery"_s},
                                                     {u"u2f-devices"_s, u"security-high"_s}};

    return plugIcons.value(plugName, u"dialog-question"_s);
}

/**
 * Capitalize the given string, by uppercasing the first letter,
 * and any letter that follows a hyphen.
 *
 * @param text the string to capitalize
 * @return the capitalized string
 */
const QString SnapBackend::capitalize(const QString &text)
{
    QString title = text;
    title[0] = title[0].toUpper();
    for (int i = 1; i < title.length(); ++i) {
        if (title[i - 1] == u"-"_s) {
            title[i - 1] = QChar::Space;
            title[i] = title[i].toUpper();
        }
    }
    return title;
}