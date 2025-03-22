/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.kcm.snappermissions 1.0

RowLayout {
    id: header
    required property KCMSnap snap
    readonly property var icon: snap.icon
    readonly property string title: snap.title
    readonly property string subtitle: snap.description
    readonly property string version: snap.version
    readonly property bool invokable: snap.invokable
    readonly property string desktopFile: snap.desktopFile

    spacing: Kirigami.Units.largeSpacing

    Kirigami.Icon {
        // Fallback doesn't kick in when source is an empty string/url
        source: header.icon

        Layout.alignment: Qt.AlignCenter
        Layout.preferredWidth: Kirigami.Units.iconSizes.large
        Layout.preferredHeight: Kirigami.Units.iconSizes.large

        // RowLayout is incapable of paddings, so use margins on both child items instead.
        Layout.margins: Kirigami.Units.largeSpacing
    }
    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        Layout.margins: Kirigami.Units.largeSpacing
        Layout.leftMargin: 0

        Kirigami.Heading {
            text: header.title
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        Kirigami.Heading {
            text: header.subtitle
            type: Kirigami.Heading.Secondary
            level: 3
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
}
