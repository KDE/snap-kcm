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
        text: i18nc("select an app", "Select an application from the list to view its permissions here")
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        anchors.centerIn: parent
        visible: snap === null
    }

    Kirigami.Separator {
        anchors.left: parent.left
        height: parent.height
    }

    Component {
        id: headerLoader
        Loader {
            active: root.snap !== null
            sourceComponent: PermissionHeader {
                snap: root.snap
            }
        }
    }

    ListView {
        id: listView
        header: headerLoader
        model: root.snap === null ? null : root.snap.plugs
        currentIndex: -1
        spacing: Kirigami.Units.largeSpacing
        delegate: PlugDelegate {
            required property var modelData
            plug: modelData
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
        }
    }

    view: listView

    ErrorDialog {
        id: errorOverlay
    }
}
