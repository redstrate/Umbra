// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>
#include <qcorotask.h>

class AccountConfig;

class Account : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from AccountManager")

    Q_PROPERTY(AccountConfig *config READ config CONSTANT)
    Q_PROPERTY(bool needsPassword READ needsPassword NOTIFY needsPasswordChanged)
    Q_PROPERTY(bool isWinePrefixDefault READ isWinePrefixDefault NOTIFY winePrefixChanged)

public:
    explicit Account(const QString &key, QObject *parent = nullptr);
    ~Account() override;

    [[nodiscard]] QString uuid() const;

    Q_INVOKABLE QString getPassword();
    void setPassword(const QString &password);

    [[nodiscard]] bool needsPassword() const;
    bool isWinePrefixDefault() const;

    AccountConfig *config() const;

Q_SIGNALS:
    bool needsPasswordChanged();
    void winePrefixChanged();
    void autoConfigurationResult(const QString &title, const QString &subtitle);

private:
    QCoro::Task<> fetchPassword();

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
};
