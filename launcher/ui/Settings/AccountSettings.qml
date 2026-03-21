// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.umbra

FormCard.FormCardPage {
    id: page

    property var account

    title: i18nc("@title:window", "Edit Account")

    Connections {
        target: page.account

        function onAutoConfigurationResult(title: string, subtitle: string): void {
            configurationPrompt.title = title;
            configurationPrompt.subtitle = subtitle;
            configurationPrompt.visible = true;
            page.reloadServerConfiguration();
        }
    }

    // Needed because Kirigami Add-ons overwrites our bindings with static values
    function reloadServerConfiguration(): void {
        loginServerDelegate.text = page.account.config.loginServer;
        bootPatchServerDelegate.text = page.account.config.bootPatchServer;
        gamePatchServerDelegate.text = page.account.config.gamePatchServer;
        gameServerDelegate.text = page.account.config.lobbyServer;
    }

    Component.onCompleted: reloadServerConfiguration()

    actions: [
        Kirigami.Action {
            text: i18n("Delete Account…")
            tooltip: !enabled ? i18n("Cannot delete the only account.") : ""
            icon.name: "delete"
            enabled: LauncherCore.accountManager.canDelete(page.account)

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
                id: loginAction
                text: i18n("Login")
                icon.name: "password-copy-symbolic"
            },
            Kirigami.Action {
                id: serverAction
                text: i18n("Server")
                icon.name: "network-server-symbolic"
            }
        ]

        Component.onCompleted: actions[0].checked = true
    }

    FormCard.FormCard {
        visible: generalAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: usernameDelegate

            label: i18n("Username")
            text: page.account.config.name
            onTextChanged: {
                page.account.config.name = text;
                page.account.config.save();
            }
        }
    }

    FormCard.FormCard {
        visible: loginAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: rememberPasswordDelegate

            text: i18n("Remember password")
            description: i18n("Stores the password on the device, using it's existing secure credential storage.")
            checked: page.account.config.rememberPassword
            onCheckedChanged: {
                page.account.config.rememberPassword = checked;
                page.account.config.save();
            }
        }
    }

    FormCard.FormCard {
        visible: serverAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: serverUrlDelegate

            label: i18n("Server URL")
        }

        FormCard.FormDelegateSeparator {
            above: serverUrlDelegate
            below: downloadConfigDelegate
        }

        FormCard.FormButtonDelegate {
            id: downloadConfigDelegate

            icon.name: "download-symbolic"
            text: i18nc("@action:button", "Download Configuration")
            enabled: serverUrlDelegate.text.length > 0

            onClicked: LauncherCore.downloadServerConfiguration(page.account, serverUrlDelegate.text)
        }
    }

    FormCard.FormCard {
        visible: serverAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormTextFieldDelegate {
            id: preferredProtocolDelegate

            label: i18n("Preferred Protocol")
            text: page.account.config.preferredProtocol
            onTextChanged: {
                page.account.config.preferredProtocol = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: preferredProtocolDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: loginServerDelegate

            label: i18n("Login Server")
            onTextChanged: {
                page.account.config.loginServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: loginServerDelegate
            below: bootPatchServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: bootPatchServerDelegate

            label: i18n("Boot Patch Server")
            onTextChanged: {
                page.account.config.bootPatchServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: bootPatchServerDelegate
            below: gamePatchServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gamePatchServerDelegate

            label: i18n("Game Patch Server")
            onTextChanged: {
                page.account.config.gamePatchServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: gamePatchServerDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gameServerDelegate

            label: i18n("Lobby Server")
            onTextChanged: {
                page.account.config.lobbyServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: gameServerDelegate
            below: winePrefixPathDelegate
        }

        FormFolderDelegate {
            id: winePrefixPathDelegate

            text: i18n("Wine Prefix Folder")
            folder: page.account.config.winePrefixPath
            displayText: page.account.isWinePrefixDefault ? i18n("Default Location") : folder

            onAccepted: (path) => {
                page.account.config.winePrefixPath = path;
                page.account.config.save();
            }
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Account")
        subtitle: i18nc("@label", "Are you sure you want to delete this account?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay

        onAccepted: {
            LauncherCore.accountManager.deleteAccount(page.account);
            page.Window.window.pageStack.layers.pop();
        }
    }

    Kirigami.PromptDialog {
        id: configurationPrompt

        standardButtons: Kirigami.Dialog.Ok
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay
    }
}
