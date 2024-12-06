/**
 * SPDX-FileCopyrightText: 2024 Soumyadeep Ghosh <soumyadghosh@ubuntu.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kcmutils as KCMUtils
import org.kde.plasma.kcm.snappermissions 1.0

KCMUtils.ScrollViewKCM {
    id: root
    implicitWidth: Kirigami.Units.gridUnit * 20
    implicitHeight: Kirigami.Units.gridUnit * 40
    SnapBackend {
        id: backendInstance
    }
    property SnapBackend perm: backendInstance
    property string output: ""
    property string searchQuery: ""

    title: i18n("Snap Applications")
    framedView: false

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

    ListView {
        id: view
        anchors.fill: parent
        model: root.perm.snaps(root.searchQuery)
        spacing: Kirigami.Units.largeSpacing
        delegate: QQC2.ItemDelegate {
            id: delegate
            required property var modelData
            property string snapTitle: root.perm.capitalize(modelData.snap.name)
            width: ListView.view.width
            contentItem: RowLayout {
                spacing: 20
                Kirigami.Icon {
                    source: modelData.icon
                    fallback: "package-x-generic"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                }
                KD.TitleSubtitle {
                    id: appDelegates
                    title: snapTitle
                    subtitle: modelData.snap.summary
                    selected: delegate.highlighted
                    font: delegate.font
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                QQC2.Button {
                    id: invokeButton
                    text: i18n("Launch")
                    visible: root.perm.invokAble(modelData.snap)
                    onClicked: {
                        root.perm.invokeDesktopApp(modelData.snap);
                    }
                }
            }
            onClicked: {
                overlay.open();
            }

            Kirigami.OverlaySheet {
                id: overlay
                parent: view.QQC2.Overlay.overlay
                width: view.width / 2
                title: i18n("Permissions for %1", snapTitle)

                ListView {
                    id: overlayView
                    model: modelData.plugs
                    anchors.fill: parent
                    delegate: QQC2.ItemDelegate {
                        id: smallDelegate
                        width: parent.width
                        property string plugName: modelData.name
                        property string plugInterface: modelData.interface
                        property int plugCount: modelData.connectedSlotCount
                        property string slotInterface: modelData.interface
                        property string plugSnap: modelData.snap
                        property string plugLabel: root.perm.plugLabel(modelData)
                        contentItem: RowLayout {
                            Kirigami.Icon {
                                source: root.perm.plugIcon(plugInterface)
                                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                            }

                            KD.TitleSubtitle {
                                title: root.perm.capitalize(plugInterface)
                                subtitle: plugLabel
                                selected: delegate.highlighted
                                font: delegate.font
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.margins: Kirigami.Units.mediumSpacing
                            }

                            QQC2.Switch {
                                checked: plugCount === 1 || modelData.checked === true
                                Layout.margins: Kirigami.Units.mediumSpacing
                                onClicked: {
                                    if (checked) {
                                        output = root.perm.connectPlug(plugSnap, plugName, "snapd", slotInterface);
                                    }
                                    if (!checked) {
                                        output = root.perm.disconnectPlug(plugSnap, plugName, "snapd", slotInterface);
                                    }
                                    if (output !== "") {
                                        errorOverlay.open();
                                        toggle();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Kirigami.OverlaySheet {
        id: errorOverlay
        property bool copyButtonEnabled: true
        parent: view.QQC2.Overlay.overlay
        width: view.width / 2
        title: i18n("Error")
        visible: output !== ""
        bottomPadding: 10

        Kirigami.FormLayout {
            QQC2.Label {
                Kirigami.FormData.label: ""
                text: i18n("snapd error: %1", output)
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
            }
        }
    }
}
