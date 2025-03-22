/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.plasma.kcm.snappermissions 1.0

KCMUtils.ScrollViewKCM {
    id: root
    property KCMSnap snap: null
    title: snap === null ? i18n("Permissions") : i18n("Permissions for %1", snap.title)
    implicitWidth: Kirigami.Units.gridUnit * 15
    framedView: false
    Kirigami.PlaceholderMessage {
        text: i18n("Select an application from the list to view its permissions here")
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        anchors.centerIn: parent
        visible: snap === null
    }

    Kirigami.Separator {
        anchors.left: parent.left
        height: parent.height
    }
    Component {
        id: headerComponent

        RowLayout {
            id: header

            readonly property url icon: root.snap.icon
            readonly property string title: root.snap.title
            readonly property string subtitle: root.snap.description
            readonly property string version: root.snap.version
            readonly property bool invokable: root.snap.invokable
            readonly property string desktopFile: root.snap.desktopFile

            spacing: 0

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
    }

    ListView {
        id: listView
        header: headerComponent
        model: root.snap.plugs
        currentIndex: -1
        spacing: Kirigami.Units.largeSpacing
        delegate: PlugDelegate {
            required property var modelData
            plug: modelData
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
        }
    }

    view: root.snap == null ? null : listView

    ErrorDialog {
        id: errorOverlay
    }
}
