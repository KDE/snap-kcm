/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.plasma.kcm.snappermissions 1.0

QQC2.ItemDelegate {
    id: smallDelegate
    required property KCMPlug plug
    readonly property int plugCount: plug.connectedSlotCount
    readonly property string plugIcon: plug.plugIcon
    readonly property string plugLabel: plug.plugLabel
    readonly property string plugTitle: plug.title
    contentItem: RowLayout {
        Kirigami.Icon {
            source: plugIcon
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        }

        KD.TitleSubtitle {
            title: plugTitle
            subtitle: plugLabel
            selected: smallDelegate.highlighted
            font: smallDelegate.font
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Kirigami.Units.mediumSpacing
        }

        QQC2.ComboBox {
            id: slotList
            Layout.alignment: Qt.AlignHCenter
            enabled: !plugToggle.checked
            Layout.margins: Kirigami.Units.mediumSpacing
            model: plug.slotSnaps
            visible: count > 1
        }

        QQC2.Switch {
            id: plugToggle
            property string slotSnap: slotList.currentText
            checked: plugCount === 1 || plugToggle.checked === true
            Layout.margins: Kirigami.Units.mediumSpacing
            onClicked: {
                var slotSnap = checked ? slotList.currentText : plug.connectedSlotSnap;
                plug.changePermission(checked, slotSnap);
            }
        }
    }

    Connections {
        target: plug
        function onErrorLogChanged(message: string) {
            plugToggle.toggle();
            errorOverlay.errorLog = message;
            errorOverlay.open();
        }
    }
}
