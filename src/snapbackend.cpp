/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "snapbackend.h"
#include <KLocalizedString>
#include <QDBusInterface>
#include <QtQml>

SnapBackend::SnapBackend()
{
    // Initalize locals
    QList<QSnapdSnap *> loadedSnaps;
    QList<QSnapdPlug *> loadedPlugs;
    QList<QSnapdSlot *> loadedSlots;
    QSnapdGetSnapsRequest *reqGetSnaps{m_client.getSnaps()};
    QSnapdGetConnectionsRequest *reqGetConnections{m_client.getConnections(QSnapdClient::GetConnectionsFlag::SelectAll)};

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
    QStringList hiddenPlugs = {QStringLiteral("x11"),
                               QStringLiteral("content"),
                               QStringLiteral("desktop"),
                               QStringLiteral("desktop-legacy"),
                               QStringLiteral("mir"),
                               QStringLiteral("wayland"),
                               QStringLiteral("unity7"),
                               QStringLiteral("opengl")};

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
            if (plug->snap() == snap->name() && (!plug->hasAttribute(QStringLiteral("content")) && !(hiddenPlugs.contains(plug->name())))) {
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
    QDBusInterface interface(QStringLiteral("io.snapcraft.Launcher"),
                             QStringLiteral("/io/snapcraft/PrivilegedDesktopLauncher"),
                             QStringLiteral("io.snapcraft.PrivilegedDesktopLauncher"),
                             QDBusConnection::sessionBus());
    interface.call(QStringLiteral("OpenDesktopEntry"), desktop);
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
        {QStringLiteral("account-control"), i18n("Add user accounts and change passwords")},
        {QStringLiteral("accounts-service"), i18n("Allows communication with the accounts service, such as GNOME Online Accounts")},
        {QStringLiteral("alsa"), i18n("Play and record sound")},
        {QStringLiteral("appstream-metadata"), i18n("Allows access to AppStream metadata")},
        {QStringLiteral("audio-playback"), i18n("Play audio")},
        {QStringLiteral("audio-record"), i18n("Record audio")},
        {QStringLiteral("avahi-control"), i18n("Advertise services over the local network")},
        {QStringLiteral("avahi-observe"), i18n("Detect network devices using mDNS/DNS-SD (Bonjour/zeroconf)")},
        {QStringLiteral("bluetooth-control"), i18n("Access bluetooth hardware directly")},
        {QStringLiteral("bluez"), i18n("Use bluetooth devices")},
        {QStringLiteral("browser-support"), i18n("Use functions essential for Web browsers")},
        {QStringLiteral("camera"), i18n("Use your camera")},
        {QStringLiteral("cups"), i18n("Access to the CUPS socket for printing")},
        {QStringLiteral("cups-control"), i18n("Print documents")},
        {QStringLiteral("joystick"), i18n("Use any connected joystick")},
        {QStringLiteral("desktop-launch"), i18n("Identify and launch desktop apps from other snaps")},
        {QStringLiteral("docker"), i18n("Allow connecting to the Docker service")},
        {QStringLiteral("firewall-control"), i18n("Configure network firewall")},
        {QStringLiteral("fuse-support"), i18n("Setup and use privileged FUSE filesystems")},
        {QStringLiteral("fwupd"), i18n("Update firmware on this device")},
        {QStringLiteral("gsettings"), i18n("Provides access to any GSettings item for current user")},
        {QStringLiteral("hardware-observe"), i18n("Access hardware information")},
        {QStringLiteral("hardware-random-control"), i18n("Provide entropy to hardware random number generator")},
        {QStringLiteral("hardware-random-observe"), i18n("Use hardware-generated random numbers")},
        {QStringLiteral("home"), i18n("Access files in your home folder")},
        {QStringLiteral("libvirt"), i18n("Access libvirt service")},
        {QStringLiteral("locale-control"), i18n("Change system language and region settings")},
        {QStringLiteral("location-control"), i18n("Change location settings and providers")},
        {QStringLiteral("location-observe"), i18n("Access your location")},
        {QStringLiteral("log-observe"), i18n("Read system and application logs")},
        {QStringLiteral("lxd"), i18n("Access LXD service")},
        {QStringLiteral("lxd-support"), i18n("Allows operating as the LXD service")},
        {QStringLiteral("modem-manager"), i18n("Use and configure modems")},
        {QStringLiteral("mount-observe"), i18n("Read system mount information and disk quotas")},
        {QStringLiteral("mpris"), i18n("Control music and video players")},
        {QStringLiteral("network"), i18n("Access the internet")},
        {QStringLiteral("network-bind"), i18n("Allows operating as a network service, enabling snap to run a server")},
        {QStringLiteral("network-control"), i18n("Change low-level network settings")},
        {QStringLiteral("network-manager"), i18n("Access the NetworkManager service to read and change network settings")},
        {QStringLiteral("network-manager-observe"), i18n("Allows observing NetworkManager settings")},
        {QStringLiteral("network-observe"), i18n("Read access to network settings")},
        {QStringLiteral("network-setup-control"), i18n("Change network settings")},
        {QStringLiteral("network-setup-observe"), i18n("Read network settings")},
        {QStringLiteral("network-status"), i18n("Access the NetworkStatus service")},
        {QStringLiteral("ofono"), i18n("Access the ofono service to read and change network settings for mobile telephony")},
        {QStringLiteral("openvswitch"), i18n("Control Open vSwitch hardware")},
        {QStringLiteral("optical-drive"), i18n("Read from CD/DVD")},
        {QStringLiteral("password-manager-service"), i18n("Read, add, change, or remove saved passwords")},
        {QStringLiteral("packagekit-control"), i18n("Control the PackageKit service")},
        {QStringLiteral("pcscd"), i18n("Permits communication with PCSD smart card daemon")},
        {QStringLiteral("ppp"), i18n("Access pppd and ppp devices for configuring Point-to-Point Protocol connections")},
        {QStringLiteral("process-control"), i18n("Pause or end any process on the system")},
        {QStringLiteral("pulseaudio"), i18n("Play and record sound")},
        {QStringLiteral("raw-usb"), i18n("Access USB hardware directly")},
        {QStringLiteral("removable-media"), i18n("Read/write files on removable storage devices")},
        {QStringLiteral("screen-inhibit-control"), i18n("Prevent screen sleep/lock")},
        {QStringLiteral("serial-port"), i18n("Access serial port hardware")},
        {QStringLiteral("shared-memory"), i18n("Enables two snaps to access the same shared memory")},
        {QStringLiteral("shutdown"), i18n("Restart or power off the device")},
        {QStringLiteral("snapd-control"), i18n("Install, remove and configure software (snaps)")},
        {QStringLiteral("ssh-keys"), i18n("Access SSH private and public keys")},
        {QStringLiteral("ssh-public-keys"), i18n("Access SSH public keys")},
        {QStringLiteral("storage-framework-service"), i18n("Access Storage Framework service")},
        {QStringLiteral("system-observe"), i18n("Read process and system information")},
        {QStringLiteral("system-packages-doc"), i18n("Access system documentation in /usr/share/doc")},
        {QStringLiteral("system-trace"), i18n("Monitor and control any running program")},
        {QStringLiteral("time-control"), i18n("Change the date and time")},
        {QStringLiteral("timeserver-control"), i18n("Change time server settings")},
        {QStringLiteral("timezone-control"), i18n("Change the time zone")},
        {QStringLiteral("udisks2"), i18n("Access the UDisks2 service for configuring disks and removable media")},
        {QStringLiteral("upower-observe"), i18n("Access energy usage data")},
        {QStringLiteral("u2f-devices"), i18n("Read/write access to U2F devices exposed")}};

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
    static const QMap<QString, QString> plugIcons = {{QStringLiteral("account-control"), QStringLiteral("user-identity")},
                                                     {QStringLiteral("accounts-service"), QStringLiteral("user-identity")},
                                                     {QStringLiteral("alsa"), QStringLiteral("audio-card")},
                                                     {QStringLiteral("appstream-metadata"), QStringLiteral("package-x-generic")},
                                                     {QStringLiteral("avahi-control"), QStringLiteral("network-wired")},
                                                     {QStringLiteral("avahi-observe"), QStringLiteral("network-wired")},
                                                     {QStringLiteral("audio-playback"), QStringLiteral("audio-headphones")},
                                                     {QStringLiteral("audio-record"), QStringLiteral("microphone-sensitivity-high")},
                                                     {QStringLiteral("avahi-observe"), QStringLiteral("network-workgroup")},
                                                     {QStringLiteral("bluetooth-control"), QStringLiteral("preferences-system-bluetooth")},
                                                     {QStringLiteral("bluez"), QStringLiteral("preferences-system-bluetooth")},
                                                     {QStringLiteral("browser-support"), QStringLiteral("security-high")},
                                                     {QStringLiteral("camera"), QStringLiteral("camera-photo")},
                                                     {QStringLiteral("cups"), QStringLiteral("printer")},
                                                     {QStringLiteral("cups-control"), QStringLiteral("printer")},
                                                     {QStringLiteral("joystick"), QStringLiteral("input-gaming")},
                                                     {QStringLiteral("docker"), QStringLiteral("docker")},
                                                     {QStringLiteral("desktop"), QStringLiteral("application-x-desktop")},
                                                     {QStringLiteral("desktop-legacy"), QStringLiteral("application-x-desktop")},
                                                     {QStringLiteral("desktop-launch"), QStringLiteral("kt-start")},
                                                     {QStringLiteral("firewall-control"), QStringLiteral("preferences-system-firewall")},
                                                     {QStringLiteral("fuse-support"), QStringLiteral("folder-lock")},
                                                     {QStringLiteral("fwupd"), QStringLiteral("software-update-available")},
                                                     {QStringLiteral("gsettings"), QStringLiteral("settings-configure")},
                                                     {QStringLiteral("hardware-observe"), QStringLiteral("preferences-system")},
                                                     {QStringLiteral("hardware-random-control"), QStringLiteral("preferences-desktop-random")},
                                                     {QStringLiteral("hardware-random-observe"), QStringLiteral("preferences-desktop-random")},
                                                     {QStringLiteral("home"), QStringLiteral("user-home")},
                                                     {QStringLiteral("libvirt"), QStringLiteral("preferences-system-network")},
                                                     {QStringLiteral("locale-control"), QStringLiteral("preferences-desktop-locale")},
                                                     {QStringLiteral("location-control"), QStringLiteral("preferences-location")},
                                                     {QStringLiteral("location-observe"), QStringLiteral("preferences-location")},
                                                     {QStringLiteral("log-observe"), QStringLiteral("document-edit")},
                                                     {QStringLiteral("lxd"), QStringLiteral("utilities-terminal")},
                                                     {QStringLiteral("lxd-support"), QStringLiteral("utilities-terminal")},
                                                     {QStringLiteral("modem-manager"), QStringLiteral("network-mobile")},
                                                     {QStringLiteral("mount-observe"), QStringLiteral("drive-harddisk")},
                                                     {QStringLiteral("mpris"), QStringLiteral("multimedia-player")},
                                                     {QStringLiteral("network"), QStringLiteral("network-connect")},
                                                     {QStringLiteral("network-bind"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("network-control"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("network-manager"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("network-observe"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("network-manager"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("network-manager-observe"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("network-status"), QStringLiteral("network-manager")},
                                                     {QStringLiteral("ofono"), QStringLiteral("modem")},
                                                     {QStringLiteral("openvswitch"), QStringLiteral("network")},
                                                     {QStringLiteral("optical-drive"), QStringLiteral("drive-optical")},
                                                     {QStringLiteral("packagekit-control"), QStringLiteral("kpackagekit-updates")},
                                                     {QStringLiteral("password-manager-service"), QStringLiteral("dialog-password")},
                                                     {QStringLiteral("pcscd"), QStringLiteral("secure-card")},
                                                     {QStringLiteral("ppp"), QStringLiteral("network")},
                                                     {QStringLiteral("process-control"), QStringLiteral("system-run")},
                                                     {QStringLiteral("pulseaudio"), QStringLiteral("audio-volume-high")},
                                                     {QStringLiteral("raw-usb"), QStringLiteral("drive-removable-media")},
                                                     {QStringLiteral("removable-media"), QStringLiteral("drive-removable-media")},
                                                     {QStringLiteral("screen-inhibit-control"), QStringLiteral("video-display")},
                                                     {QStringLiteral("serial-port"), QStringLiteral("port-serial")},
                                                     {QStringLiteral("shared-memory"), QStringLiteral("memory")},
                                                     {QStringLiteral("shutdown"), QStringLiteral("system-shutdown")},
                                                     {QStringLiteral("snapd-control"), QStringLiteral("folder-snap")},
                                                     {QStringLiteral("ssh-keys"), QStringLiteral("network-server")},
                                                     {QStringLiteral("ssh-public-keys"), QStringLiteral("network-server")},
                                                     {QStringLiteral("storage-framework-service"), QStringLiteral("drive-harddisk")},
                                                     {QStringLiteral("system-observe"), QStringLiteral("utilities-system-monitor")},
                                                     {QStringLiteral("system-packages-doc"), QStringLiteral("documentation")},
                                                     {QStringLiteral("system-trace"), QStringLiteral("document-preview")},
                                                     {QStringLiteral("time-control"), QStringLiteral("preferences-system-time")},
                                                     {QStringLiteral("timeserver-control"), QStringLiteral("preferences-system-time")},
                                                     {QStringLiteral("timezone-control"), QStringLiteral("preferences-system-time")},
                                                     {QStringLiteral("udisks2"), QStringLiteral("drive-harddisk")},
                                                     {QStringLiteral("upower-observe"), QStringLiteral("battery")},
                                                     {QStringLiteral("u2f-devices"), QStringLiteral("security-high")}};

    return plugIcons.value(plugName, QStringLiteral("dialog-question"));
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
        if (title[i - 1] == QStringLiteral("-")) {
            title[i - 1] = QChar::Space;
            title[i] = title[i].toUpper();
        }
    }
    return title;
}
