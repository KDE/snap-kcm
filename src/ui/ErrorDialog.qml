/**
 * SPDX-FileCopyrightText: 2025 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: errorDialog
    property alias errorLog: errorLabel.text
    padding: Kirigami.Units.largeSpacing
    standardButtons: Kirigami.Dialog.Ok
    title: i18n("Error")
    Kirigami.SelectableLabel {
        id: errorLabel
        Kirigami.FormData.label: ""
        textFormat: Text.StyledText
        wrapMode: Text.Wrap
    }
}
