// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.umbra

FormCard.FormCardPage {
    id: page

    readonly property bool isValid: usernameField.text.length !== 0 && serverURLField.text.length !== 0
    property var profile
    property var account

    title: i18n("Add New Account")

    Connections {
        target: LauncherCore

        function onServerCheckResult(successful: bool, title: string, subtitle: string): void {
            if (successful) {
                let account = LauncherCore.accountManager.createSquareEnixAccount(usernameField.text);
                LauncherCore.downloadServerConfiguration(account, serverURLField.text);
                if (page.profile) {
                    page.profile.account = account;
                    applicationWindow().checkSetup();
                } else {
                    page.Window.window.pageStack.layers.pop();
                }
            } else {
                configurationPrompt.title = title;
                configurationPrompt.subtitle = subtitle;
                configurationPrompt.visible = true;
            }
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            description: i18n("The password will be entered on the login page. A username will be associated with this account but can always be changed later.")
        }
        FormCard.FormDelegateSeparator {
            above: helpTextDelegate
            below: usernameField
        }
        FormCard.FormTextFieldDelegate {
            id: usernameField

            label: i18n("Username")
            focus: true
        }
        FormCard.FormDelegateSeparator {
            above: usernameField
            below: serverURLField
        }
        FormCard.FormTextFieldDelegate {
            id: serverURLField

            label: i18n("Server URL")
        }
        FormCard.FormDelegateSeparator {
            above: serverURLField
            below: buttonDelegate
        }
        FormCard.FormButtonDelegate {
            id: buttonDelegate

            enabled: page.isValid
            icon.name: "list-add-symbolic"
            text: i18n("Add Account")

            onClicked: LauncherCore.checkIfServerIsValid(serverURLField.text)
        }
    }

    Kirigami.PromptDialog {
        id: configurationPrompt

        standardButtons: Kirigami.Dialog.Ok
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay
    }
}
