// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.umbra

import "../Components"

FormCard.FormCardPage {
    id: page

    property var profile

    title: i18nc("@window:title", "Edit Profile")

    actions: [
        Kirigami.Action {
            text: i18n("Delete Profile…")
            icon.name: "delete"
            enabled: LauncherCore.profileManager.canDelete(page.profile)
            tooltip: !enabled ? i18n("Cannot delete the only profile.") : ""

            onTriggered: deletePrompt.open()
        }
    ]

    header: Kirigami.NavigationTabBar {
        width: parent.width

        actions: [
            Kirigami.Action {
                id: generalAction
                text: i18n("General")
                icon.name: "configure-symbolic"
            },
            Kirigami.Action {
                id: wineAction
                text: i18n("Wine")
                visible: !LauncherCore.isWindows
                icon.name: "wine-symbolic"
            },
            Kirigami.Action {
                id: developerAction
                text: i18n("Developer")
                visible: LauncherCore.config.showDevTools
                icon.name: "applications-development-symbolic"
            }
        ]

        Component.onCompleted: actions[0].checked = true
    }

    FormCard.FormCard {
        visible: generalAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: nameDelegate

            label: i18n("Name")
            text: page.profile.config.name
            onTextChanged: {
                page.profile.config.name = text;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: nameDelegate
            below: gamePathDelegate
        }

        FormFolderDelegate {
            id: gamePathDelegate

            text: i18n("Game Folder")
            folder: page.profile.config.gamePath
            displayText: page.profile.isGamePathDefault ? i18n("Default Location") : folder

            onAccepted: (folder) => {
                page.profile.config.gamePath = folder;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: gamePathDelegate
            below: allowUpdatesDelegate
        }

        FormCard.FormSwitchDelegate {
            id: allowUpdatesDelegate

            text: i18n("Allow Updating")
            description: i18n("If unchecked, Umbra won't try to update the game automatically.")
            checked: page.profile.config.allowPatching
            onCheckedChanged: {
                page.profile.config.allowPatching = checked;
                page.profile.config.save();
            }
            visible: LauncherCore.config.showDevTools
        }

        FormCard.FormDelegateSeparator {
            above: allowUpdatesDelegate
            visible: LauncherCore.config.showDevTools
        }

        FormCard.FormTextDelegate {
            description: page.profile.versionText
        }
    }

    FormCard.FormCard {
        visible: wineAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormComboBoxDelegate {
            id: wineTypeDelegate

            text: i18n("Wine Type")
            model: [i18n("Built-in"), i18n("Custom")]
            currentIndex: page.profile.config.wineType
            onCurrentIndexChanged: {
                page.profile.config.wineType = currentIndex;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: wineTypeDelegate
            below: winePathDelegate
        }

        FormFileDelegate {
            id: winePathDelegate

            text: i18n("Wine Executable")
            file: page.profile.winePath
            visible: page.profile.config.wineType !== Profile.BuiltIn

            onAccepted: (path) => {
                page.profile.winePath = path;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: winePathDelegate
            below: wineVersionDelegate
        }

        FormCard.FormTextDelegate {
            id: wineVersionDelegate

            description: page.profile.wineVersionText
        }
    }

    FormCard.FormCard {
        visible: developerAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: bootUpdateChannel

            label: i18n("Boot Update Channel")
            text: page.profile.config.bootUpdateChannel
            onTextChanged: {
                page.profile.config.bootUpdateChannel = text;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: bootUpdateChannel
            below: gameUpdateChannel
        }

        FormCard.FormTextFieldDelegate {
            id: gameUpdateChannel

            label: i18n("Game Update Channel")
            text: page.profile.config.gameUpdateChannel
            onTextChanged: {
                page.profile.config.gameUpdateChannel = text;
                page.profile.config.save();
            }
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Profile")
        subtitle: i18nc("@label", "Are you sure you want to delete this profile?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay

        QQC2.Switch {
            id: deleteFilesSwitch

            checked: true

            text: i18n("Delete Files")
        }

        onAccepted: {
            LauncherCore.profileManager.deleteProfile(page.profile, deleteFilesSwitch.checked);
            page.Window.window.pageStack.layers.pop();
        }
    }
}
