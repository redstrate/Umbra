// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <krasis.hpp>
#include <qqmlintegration.h>

class ConfigSys : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from Account")

    Q_PROPERTY(uint32_t upnpPortMapping MEMBER m_upnpPortMapping NOTIFY configChanged)
    Q_PROPERTY(uint32_t upnpPort MEMBER m_upnpPort NOTIFY configChanged)
    Q_PROPERTY(uint32_t devices MEMBER m_devices NOTIFY configChanged)
    Q_PROPERTY(uint32_t displayMode MEMBER m_displayMode NOTIFY configChanged)
    Q_PROPERTY(uint32_t windowWidth MEMBER m_windowWidth NOTIFY configChanged)
    Q_PROPERTY(uint32_t windowHeight MEMBER m_windowHeight NOTIFY configChanged)
    Q_PROPERTY(uint32_t generalDrawingQuality MEMBER m_generalDrawingQuality NOTIFY configChanged)
    Q_PROPERTY(uint32_t backgroundDrawingQuality MEMBER m_backgroundDrawingQuality NOTIFY configChanged)
    Q_PROPERTY(uint32_t multisampling MEMBER m_multisampling NOTIFY configChanged)
    Q_PROPERTY(uint32_t shadowDetail MEMBER m_shadowDetail NOTIFY configChanged)
    Q_PROPERTY(bool ambientOcclusion MEMBER m_ambientOcclusion NOTIFY configChanged)
    Q_PROPERTY(bool depthOfField MEMBER m_depthOfField NOTIFY configChanged)
    Q_PROPERTY(bool disableCutsceneEffects MEMBER m_disableCutsceneEffects NOTIFY configChanged)
    Q_PROPERTY(uint32_t textureQuality MEMBER m_textureQuality NOTIFY configChanged)
    Q_PROPERTY(uint32_t textureFiltering MEMBER m_textureFiltering NOTIFY configChanged)
    Q_PROPERTY(bool disableSoundDevice MEMBER m_disableSoundDevice NOTIFY configChanged)
    Q_PROPERTY(bool playSoundWhenApplicationIsInBackground MEMBER m_playSoundWhenApplicationIsInBackground NOTIFY configChanged)
    Q_PROPERTY(bool enableHardwareCursor MEMBER m_enableHardwareCursor NOTIFY configChanged)

public:
    void loadFromConfigSys(krasis_ConfigSys &config);
    void writeToConfigSys(const QString &screenshotDir, const QString &path);

Q_SIGNALS:
    void configChanged();

private:
    uint32_t m_upnpPortMapping = 0;
    uint32_t m_upnpPort = 0;
    uint32_t m_devices = 0;
    uint32_t m_displayMode = 0;
    uint32_t m_windowWidth = 0;
    uint32_t m_windowHeight = 0;
    uint32_t m_generalDrawingQuality = 0;
    uint32_t m_backgroundDrawingQuality = 0;
    uint32_t m_multisampling = 0;
    uint32_t m_shadowDetail = 0;
    bool m_ambientOcclusion = false;
    bool m_depthOfField = false;
    bool m_disableCutsceneEffects = false;
    uint32_t m_textureQuality = 0;
    uint32_t m_textureFiltering = 0;
    bool m_disableSoundDevice = 0;
    bool m_playSoundWhenApplicationIsInBackground = 0;
    bool m_enableHardwareCursor = false;
};
