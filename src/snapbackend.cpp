/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "snapbackend.h"
#include <KLocalizedString>
#include <QDBusInterface>

using namespace Qt::Literals::StringLiterals;

SnapBackend::SnapBackend()
{
    // Initalize locals
    QList<QSnapdSnap *> loadedSnaps;
    QList<QSnapdPlug *> loadedPlugs;
    QList<QSnapdSlot *> loadedSlots;
    QScopedPointer<QSnapdGetSnapsRequest> reqGetSnaps{m_client.getSnaps()};
    QScopedPointer<QSnapdGetConnectionsRequest> reqGetConnections{m_client.getConnections(QSnapdClient::GetConnectionsFlag::SelectAll)};

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
        for (int i = 0; i < reqGetConnections->slotCount(); ++i) {
            loadedSlots.append(reqGetConnections->slot(i));
        }
    }
    // Get snaps and their associated plugs and slots
    for (QSnapdSnap *snap : loadedSnaps) {
        QList<QSnapdPlug *> plugsForSnap;
        QList<QSnapdSlot *> slotsForSnap;
        for (QSnapdPlug *plug : loadedPlugs) {
            if (plug->snap() == snap->name() && (!plug->hasAttribute(u"content"_s) && !(hiddenPlugs.contains(plug->name())))) {
                plugsForSnap.append(plug);
            }
        }
        for (QSnapdSlot *slot : loadedSlots) {
            if (slot->snap() == snap->name()) {
                slotsForSnap.append(slot);
            }
        }
        m_snaps.append(new KCMSnap(snap, plugsForSnap, slotsForSnap));
    }
    // Print results
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
        if (snap->snap()->name().contains(filter, Qt::CaseInsensitive)) {
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
 * Returns true if the given snap has any app with a desktop file, false otherwise.
 *
 * This is used to determine if a snap can be launched from the kcm.
 *
 * @param snap the snap to check
 */
bool SnapBackend::invokAble(QSnapdSnap *snap)
{
    bool invokable = false;
    for (int i = 0; i < snap->appCount(); i++) {
        auto app = snap->app(i);
        if (!app->desktopFile().isEmpty()) {
            invokable = true;
        }
    }
    return invokable;
}

/**
 * Launches the given snap if it has a desktop file.
 *
 * This is used when the user wants to launch a snap from the kcm.
 *
 * @param snap the snap to launch
 */
void SnapBackend::invokeDesktopApp(QSnapdSnap *snap) const
{
    QString desktop;
    for (int i = 0; i < snap->appCount(); i++) {
        QSnapdApp *app = snap->app(i);
        if (app->name() == snap->name()) {
            desktop = app->desktopFile().mid(app->desktopFile().lastIndexOf(QLatin1Char('/')) + 1);
        }
    }
    QDBusInterface interface(u"io.snapcraft.Launcher"_s,
                             u"/io/snapcraft/PrivilegedDesktopLauncher"_s,
                             u"io.snapcraft.PrivilegedDesktopLauncher"_s,
                             QDBusConnection::sessionBus());
    interface.call(u"OpenDesktopEntry"_s, desktop);
}

/**
 * Returns a human-readable label for the given plug interface.
 *
 * This function maps a plug's interface name to a corresponding
 * description that explains what access or functionality the plug
 * provides. If the interface name is not recognized, the function
 * returns the plug's name in lowercase as a fallback.
 *
 * @param plug A pointer to the QSnapdPlug object representing the plug
 *             for which the label is requested.
 * @return A QString containing the user-friendly label or the plug
 *         name in lowercase if the interface is not found.
 */
const QString SnapBackend::plugLabel(const QSnapdPlug *plug)
{
    static const QMap<QString, QString> interfaceLabels = {
        {u"account-control"_s, i18n("Add user accounts and change passwords")},
        {u"accounts-service"_s, i18n("Allows communication with the accounts service, such as GNOME Online Accounts")},
        {u"alsa"_s, i18n("Play and record sound")},
        {u"appstream-metadata"_s, i18n("Allows access to AppStream metadata")},
        {u"audio-playback"_s, i18n("Play audio")},
        {u"audio-record"_s, i18n("Record audio")},
        {u"avahi-control"_s, i18n("Advertise services over the local network")},
        {u"avahi-observe"_s, i18n("Detect network devices using mDNS/DNS-SD (Bonjour/zeroconf)")},
        {u"bluetooth-control"_s, i18n("Access bluetooth hardware directly")},
        {u"bluez"_s, i18n("Use bluetooth devices")},
        {u"browser-support"_s, i18n("Use functions essential for Web browsers")},
        {u"camera"_s, i18n("Use your camera")},
        {u"cups"_s, i18n("Access to the CUPS socket for printing")},
        {u"cups-control"_s, i18n("Print documents")},
        {u"joystick"_s, i18n("Use any connected joystick")},
        {u"desktop-launch"_s, i18n("Identify and launch desktop apps from other snaps")},
        {u"docker"_s, i18n("Allow connecting to the Docker service")},
        {u"firewall-control"_s, i18n("Configure network firewall")},
        {u"fuse-support"_s, i18n("Setup and use privileged FUSE filesystems")},
        {u"fwupd"_s, i18n("Update firmware on this device")},
        {u"gsettings"_s, i18n("Provides access to any GSettings item for current user")},
        {u"hardware-observe"_s, i18n("Access hardware information")},
        {u"hardware-random-control"_s, i18n("Provide entropy to hardware random number generator")},
        {u"hardware-random-observe"_s, i18n("Use hardware-generated random numbers")},
        {u"home"_s, i18n("Access files in your home folder")},
        {u"libvirt"_s, i18n("Access libvirt service")},
        {u"locale-control"_s, i18n("Change system language and region settings")},
        {u"location-control"_s, i18n("Change location settings and providers")},
        {u"location-observe"_s, i18n("Access your location")},
        {u"log-observe"_s, i18n("Read system and application logs")},
        {u"lxd"_s, i18n("Access LXD service")},
        {u"lxd-support"_s, i18n("Allows operating as the LXD service")},
        {u"modem-manager"_s, i18n("Use and configure modems")},
        {u"mount-observe"_s, i18n("Read system mount information and disk quotas")},
        {u"mpris"_s, i18n("Control music and video players")},
        {u"network"_s, i18n("Access the internet")},
        {u"network-bind"_s, i18n("Allows operating as a network service, enabling snap to run a server")},
        {u"network-control"_s, i18n("Change low-level network settings")},
        {u"network-manager"_s, i18n("Access the NetworkManager service to read and change network settings")},
        {u"network-manager-observe"_s, i18n("Allows observing NetworkManager settings")},
        {u"network-observe"_s, i18n("Read access to network settings")},
        {u"network-setup-control"_s, i18n("Change network settings")},
        {u"network-setup-observe"_s, i18n("Read network settings")},
        {u"network-status"_s, i18n("Access the NetworkStatus service")},
        {u"ofono"_s, i18n("Access the ofono service to read and change network settings for mobile telephony")},
        {u"openvswitch"_s, i18n("Control Open vSwitch hardware")},
        {u"optical-drive"_s, i18n("Read from CD/DVD")},
        {u"password-manager-service"_s, i18n("Read, add, change, or remove saved passwords")},
        {u"packagekit-control"_s, i18n("Control the PackageKit service")},
        {u"pcscd"_s, i18n("Permits communication with PCSD smart card daemon")},
        {u"ppp"_s, i18n("Access pppd and ppp devices for configuring Point-to-Point Protocol connections")},
        {u"process-control"_s, i18n("Pause or end any process on the system")},
        {u"pulseaudio"_s, i18n("Play and record sound")},
        {u"raw-usb"_s, i18n("Access USB hardware directly")},
        {u"removable-media"_s, i18n("Read/write files on removable storage devices")},
        {u"screen-inhibit-control"_s, i18n("Prevent screen sleep/lock")},
        {u"serial-port"_s, i18n("Access serial port hardware")},
        {u"shared-memory"_s, i18n("Enables two snaps to access the same shared memory")},
        {u"shutdown"_s, i18n("Restart or power off the device")},
        {u"snapd-control"_s, i18n("Install, remove and configure software (snaps)")},
        {u"ssh-keys"_s, i18n("Access SSH private and public keys")},
        {u"ssh-public-keys"_s, i18n("Access SSH public keys")},
        {u"storage-framework-service"_s, i18n("Access Storage Framework service")},
        {u"system-observe"_s, i18n("Read process and system information")},
        {u"system-packages-doc"_s, i18n("Access system documentation in /usr/share/doc")},
        {u"system-trace"_s, i18n("Monitor and control any running program")},
        {u"time-control"_s, i18n("Change the date and time")},
        {u"timeserver-control"_s, i18n("Change time server settings")},
        {u"timezone-control"_s, i18n("Change the time zone")},
        {u"udisks2"_s, i18n("Access the UDisks2 service for configuring disks and removable media")},
        {u"upower-observe"_s, i18n("Access energy usage data")},
        {u"u2f-devices"_s, i18n("Read/write access to U2F devices exposed")}};

    return interfaceLabels.value(plug->interface(), plug->name().toCaseFolded());
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
