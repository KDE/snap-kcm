/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kcmutils as KCMUtils
import org.kde.plasma.kcm.snappermissions 1.0

KCMUtils.ScrollViewKCM {
    id: root
    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 20
    Kirigami.ColumnView.fillWidth: false
    SnapBackend {
        id: backendInstance
    }
    property SnapBackend perm: backendInstance
    property string output: ""
    property string searchQuery: ""

    title: i18n("Snap Applications")
    framedView: false

    function changeSnap(snap) {
        kcm.columnWidth = Kirigami.Units.gridUnit * 15;
        kcm.push("permissions.qml", {
            snap
        });
    }

    header: Kirigami.SearchField {
        id: filterField
        KeyNavigation.tab: view
        KeyNavigation.down: view
        autoAccept: false
        onTextChanged: {
            root.searchQuery = text;
        }
    }

    Kirigami.PlaceholderMessage {
        text: i18n("Install snaps to change the permissions")
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
        anchors.centerIn: parent
        visible: perm === null
    }

    Kirigami.Separator {
        anchors.left: parent.left
        height: parent.height
    }

    Component.onCompleted: {
        changeSnap();
    }

    view: ListView {
        id: view
        model: root.perm.snaps(root.searchQuery)
        currentIndex: -1
        delegate: SnapDelegate {
            id: snapdelegate
            required property var modelData
            required property int index
            snap: modelData
            highlighted: ListView.isCurrentItem
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            onClicked: {
                root.changeSnap(modelData);
                view.currentIndex = index;
            }
        }
    }
}
