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

Kirigami.SubtitleDelegate {
    id: delegate
    required property KCMSnap snap
    contentItem: RowLayout {
        id: rowLayout
        spacing: Kirigami.Units.largeSpacing
        Kirigami.Icon {
            source: snap.icon
            fallback: "package-x-generic"
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        }
        KD.TitleSubtitle {
            title: snap.title
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
