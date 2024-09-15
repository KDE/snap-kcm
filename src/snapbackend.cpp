/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "snapbackend.h"
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

bool SnapBackend::comparebyName(QSnapdSnap *a, QSnapdSnap *b)
{
    return a->name() < b->name();
}

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

const QString SnapBackend::plugLabel(const QSnapdPlug *plug)
{
    static const QMap<QString, QString> interfaceLabels = {
        {QStringLiteral("account-control"), QStringLiteral("Add user accounts and change passwords")},
        {QStringLiteral("alsa"), QStringLiteral("Play and record sound")},
        {QStringLiteral("appstream-metadata"), QStringLiteral("Allows access to AppStream metadata")},
        {QStringLiteral("audio-playback"), QStringLiteral("Play audio")},
        {QStringLiteral("audio-record"), QStringLiteral("Record audio")},
        {QStringLiteral("avahi-control"), QStringLiteral("Advertise services over the local network")},
        {QStringLiteral("avahi-observe"), QStringLiteral("Detect network devices using mDNS/DNS-SD (Bonjour/zeroconf)")},
        {QStringLiteral("bluetooth-control"), QStringLiteral("Access bluetooth hardware directly")},
        {QStringLiteral("bluez"), QStringLiteral("Use bluetooth devices")},
        {QStringLiteral("browser-support"), QStringLiteral("Use functions essential for Web browsers")},
        {QStringLiteral("camera"), QStringLiteral("Use your camera")},
        {QStringLiteral("cups"), QStringLiteral("Access to the CUPS socket for printing")},
        {QStringLiteral("cups-control"), QStringLiteral("Print documents")},
        {QStringLiteral("joystick"), QStringLiteral("Use any connected joystick")},
        {QStringLiteral("desktop-launch"), QStringLiteral("Identify and launch desktop apps from other snaps")},
        {QStringLiteral("docker"), QStringLiteral("Allow connecting to the Docker service")},
        {QStringLiteral("firewall-control"), QStringLiteral("Configure network firewall")},
        {QStringLiteral("fuse-support"), QStringLiteral("Setup and use privileged FUSE filesystems")},
        {QStringLiteral("fwupd"), QStringLiteral("Update firmware on this device")},
        {QStringLiteral("gsettings"), QStringLiteral("Provides access to any GSettings item for current user")},
        {QStringLiteral("hardware-observe"), QStringLiteral("Access hardware information")},
        {QStringLiteral("hardware-random-control"), QStringLiteral("Provide entropy to hardware random number generator")},
        {QStringLiteral("hardware-random-observe"), QStringLiteral("Use hardware-generated random numbers")},
        {QStringLiteral("home"), QStringLiteral("Access files in your home folder")},
        {QStringLiteral("libvirt"), QStringLiteral("Access libvirt service")},
        {QStringLiteral("locale-control"), QStringLiteral("Change system language and region settings")},
        {QStringLiteral("location-control"), QStringLiteral("Change location settings and providers")},
        {QStringLiteral("location-observe"), QStringLiteral("Access your location")},
        {QStringLiteral("log-observe"), QStringLiteral("Read system and application logs")},
        {QStringLiteral("lxd"), QStringLiteral("Access LXD service")},
        {QStringLiteral("lxd-support"), QStringLiteral("Allows operating as the LXD service")},
        {QStringLiteral("modem-manager"), QStringLiteral("Use and configure modems")},
        {QStringLiteral("mount-observe"), QStringLiteral("Read system mount information and disk quotas")},
        {QStringLiteral("mpris"), QStringLiteral("Control music and video players")},
        {QStringLiteral("network"), QStringLiteral("Access the internet")},
        {QStringLiteral("network-bind"),
         QStringLiteral("Allows operating as a network service, enabling snap to "
                        "run a server")},
        {QStringLiteral("network-control"), QStringLiteral("Change low-level network settings")},
        {QStringLiteral("network-manager"),
         QStringLiteral("Access the NetworkManager service to read and change "
                        "network settings")},
        {QStringLiteral("network-manager-observe"), QStringLiteral("Allows observing NetworkManager settings")},
        {QStringLiteral("network-observe"), QStringLiteral("Read access to network settings")},
        {QStringLiteral("network-setup-control"), QStringLiteral("Change network settings")},
        {QStringLiteral("network-setup-observe"), QStringLiteral("Read network settings")},
        {QStringLiteral("network-status"), QStringLiteral("Access the NetworkStatus service")},
        {QStringLiteral("ofono"),
         QStringLiteral("Access the ofono service to read and change network "
                        "settings for mobile telephony")},
        {QStringLiteral("openvswitch"), QStringLiteral("Control Open vSwitch hardware")},
        {QStringLiteral("optical-drive"), QStringLiteral("Read from CD/DVD")},
        {QStringLiteral("password-manager-service"), QStringLiteral("Read, add, change, or remove saved passwords")},
        {QStringLiteral("packagekit-control"), QStringLiteral("Control the PackageKit service")},
        {QStringLiteral("pcscd"), QStringLiteral("Permits communication with PCSD smart card daemon")},
        {QStringLiteral("ppp"),
         QStringLiteral("Access pppd and ppp devices for configuring "
                        "Point-to-Point Protocol connections")},
        {QStringLiteral("process-control"), QStringLiteral("Pause or end any process on the system")},
        {QStringLiteral("pulseaudio"), QStringLiteral("Play and record sound")},
        {QStringLiteral("raw-usb"), QStringLiteral("Access USB hardware directly")},
        {QStringLiteral("removable-media"), QStringLiteral("Read/write files on removable storage devices")},
        {QStringLiteral("screen-inhibit-control"), QStringLiteral("Prevent screen sleep/lock")},
        {QStringLiteral("serial-port"), QStringLiteral("Access serial port hardware")},
        {QStringLiteral("shared-memory"), QStringLiteral("Enables two snaps to access the same shared memory")},
        {QStringLiteral("shutdown"), QStringLiteral("Restart or power off the device")},
        {QStringLiteral("snapd-control"), QStringLiteral("Install, remove and configure software (snaps)")},
        {QStringLiteral("ssh-keys"), QStringLiteral("Access SSH private and public keys")},
        {QStringLiteral("ssh-public-keys"), QStringLiteral("Access SSH public keys")},
        {QStringLiteral("storage-framework-service"), QStringLiteral("Access Storage Framework service")},
        {QStringLiteral("system-observe"), QStringLiteral("Read process and system information")},
        {QStringLiteral("system-packages-doc"), QStringLiteral("Access system documentation in /usr/share/doc")},
        {QStringLiteral("system-trace"), QStringLiteral("Monitor and control any running program")},
        {QStringLiteral("time-control"), QStringLiteral("Change the date and time")},
        {QStringLiteral("timeserver-control"), QStringLiteral("Change time server settings")},
        {QStringLiteral("timezone-control"), QStringLiteral("Change the time zone")},
        {QStringLiteral("udisks2"),
         QStringLiteral("Access the UDisks2 service for configuring disks and "
                        "removable media")},
        {QStringLiteral("upower-observe"), QStringLiteral("Access energy usage data")},
        {QStringLiteral("u2f-devices"), QStringLiteral("Read/write access to U2F devices exposed")}};

    return interfaceLabels.value(plug->interface(), plug->name().toCaseFolded());
}

const QString SnapBackend::plugIcon(const QString &plugName)
{
    static const QMap<QString, QString> plugIcons = {{QStringLiteral("account-control"), QStringLiteral("user-identity")},
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
