// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.umbra

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "General")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: closeUmbraDelegate

            text: i18n("Hide Umbra when game is launched")
            checked: LauncherCore.config.closeWhenLaunched
            onCheckedChanged: {
                LauncherCore.config.closeWhenLaunched = checked;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: closeUmbraDelegate
            below: showDevToolsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showDevToolsDelegate

            text: i18n("Show Developer Settings")
            description: i18n("Enable settings that are useful for developers and tinkerers.")
            checked: LauncherCore.config.showDevTools
            onCheckedChanged: {
                LauncherCore.config.showDevTools = checked;
                LauncherCore.config.save();
            }
        }
    }
}
