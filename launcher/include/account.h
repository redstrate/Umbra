// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>
#include <qcorotask.h>

class LauncherCore;
class AccountConfig;
class ConfigSys;

class Account : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from AccountManager")

    Q_PROPERTY(AccountConfig *config READ config CONSTANT)
    Q_PROPERTY(bool needsPassword READ needsPassword NOTIFY needsPasswordChanged)
    Q_PROPERTY(bool isWinePrefixDefault READ isWinePrefixDefault NOTIFY winePrefixChanged)
    Q_PROPERTY(ConfigSys *configSys READ configSys CONSTANT)

public:
    explicit Account(const QString &key, LauncherCore *core, QObject *parent = nullptr);
    ~Account() override;

    [[nodiscard]] QString uuid() const;

    Q_INVOKABLE QString getPassword();
    void setPassword(const QString &password);

    [[nodiscard]] bool needsPassword() const;
    bool isWinePrefixDefault() const;

    AccountConfig *config() const;
    [[nodiscard]] ConfigSys *configSys() const;

    Q_INVOKABLE void writeConfigSys();
    Q_INVOKABLE void writeConfigLng();

Q_SIGNALS:
    bool needsPasswordChanged();
    void winePrefixChanged();
    void autoConfigurationResult(const QString &title, const QString &subtitle);
    void resetConfiguration();

private:
    QCoro::Task<> fetchPassword();
    void readConfigSys();
    QString configSysPath() const;
    QString configLngPath() const;
    QDir ffxivPath() const;

    /**
     * @brief Sets a value in the keychain. This function is asynchronous.
     */
    void setKeychainValue(const QString &key, const QString &value);

    /**
     * @brief Retrieves a value from the keychain. This function is synchronous.
     */
    QCoro::Task<QString> getKeychainValue(const QString &key);

    AccountConfig *m_config;
    QString m_key;
    bool m_needsPassword = false;
    ConfigSys *m_configSys;
    LauncherCore *m_core;
};
