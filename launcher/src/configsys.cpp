// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsys.h"

#include <QDebug>
#include <QFile>
#include <config.h>

#include "umbra_log.h"

void ConfigSys::loadFromConfigSys(krasis_ConfigSys &config)
{
    m_upnpPortMapping = static_cast<int>(config.upnp_port_mapping);
    m_upnpPort = config.upnp_port;
    m_devices = config.devices;
    m_displayMode = static_cast<int>(config.display_mode);
    m_windowWidth = config.window_width;
    m_windowHeight = config.window_height;
    m_generalDrawingQuality = config.general_drawing_quality;
    m_backgroundDrawingQuality = config.background_drawing_quality;
    m_multisampling = config.multisampling;
    m_shadowDetail = config.shadow_detail;
    m_ambientOcclusion = config.ambient_occlusion;
    m_depthOfField = config.depth_of_field;
    m_disableCutsceneEffects = config.disable_cutscene_effects;
    m_textureQuality = config.texture_quality;
    m_textureFiltering = config.texture_filtering;
    m_disableSoundDevice = config.disable_sound_device;
    m_playSoundWhenApplicationIsInBackground = config.play_sound_when_application_is_in_background;
    m_enableHardwareCursor = config.enable_hardware_mouse_cursor;
}

void ConfigSys::writeToConfigSys(const QString &screenshotDir, const QString &path)
{
    const auto screenshotDirStd = screenshotDir.toStdString();

    const auto config = krasis_ConfigSys{
        .upnp_port_mapping = static_cast<UPnPPortMapping>(m_upnpPortMapping),
        .upnp_port = m_upnpPort,
        .devices = m_devices,
        .display_mode = static_cast<DisplayMode>(m_displayMode),
        .window_width = m_windowWidth,
        .window_height = m_windowHeight,
        .general_drawing_quality = m_generalDrawingQuality,
        .background_drawing_quality = m_backgroundDrawingQuality,
        .multisampling = m_multisampling,
        .shadow_detail = m_shadowDetail,
        .ambient_occlusion = m_ambientOcclusion,
        .depth_of_field = m_depthOfField,
        .disable_cutscene_effects = m_disableCutsceneEffects,
        .texture_quality = m_textureQuality,
        .texture_filtering = m_textureFiltering,
        .disable_sound_device = m_disableSoundDevice,
        .play_sound_when_application_is_in_background = m_playSoundWhenApplicationIsInBackground,
        .enable_hardware_mouse_cursor = m_enableHardwareCursor,
        .screenshot_dir = screenshotDirStd.c_str(),
    };

    auto buffer = krasis_config_sys_write_to_buffer(&config);

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
    } else {
        qCWarning(UMBRA_LOG) << "Failed to write config.sys to" << path;
    }
}
