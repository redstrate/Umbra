// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

class Account;
class ProfileConfig;

class Profile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from ProfileManager")

    Q_PROPERTY(QString winePath READ winePath WRITE setWinePath NOTIFY winePathChanged)
    Q_PROPERTY(bool isGameInstalled READ isGameInstalled NOTIFY gameInstallChanged)
    Q_PROPERTY(Account *account READ account WRITE setAccount NOTIFY accountChanged)
    Q_PROPERTY(QString wineVersionText READ wineVersionText NOTIFY wineChanged)
    Q_PROPERTY(QString subtitle READ subtitle NOTIFY gameInstallChanged)
    Q_PROPERTY(ProfileConfig *config READ config CONSTANT)
    Q_PROPERTY(bool isGamePathDefault READ isGamePathDefault NOTIFY gameInstallChanged)
    Q_PROPERTY(QString versionText READ versionText NOTIFY versionTextChanged)

public:
    explicit Profile(const QString &key, QObject *parent = nullptr);
    ~Profile() override;

    enum WineType {
        BuiltIn,
        Custom
    };
    Q_ENUM(WineType)

    [[nodiscard]] QString uuid() const;

    [[nodiscard]] QString winePath() const;
    void setWinePath(const QString &path);

    [[nodiscard]] Account *account() const;
    void setAccount(Account *account);

    [[nodiscard]] QString wineVersionText() const;

    [[nodiscard]] bool isGameInstalled() const;
    [[nodiscard]] bool isWineInstalled() const;

    [[nodiscard]] QString bootVersion() const;
    [[nodiscard]] QString baseGameVersion() const;

    [[nodiscard]] QString compatibilityToolVersion() const;
    void setCompatibilityToolVersion(const QString &version);

    [[nodiscard]] QString subtitle() const;

    ProfileConfig *config() const;
    bool isGamePathDefault() const;

    [[nodiscard]] QString versionText() const;
    void readGameVersion();

Q_SIGNALS:
    void gameInstallChanged();
    void winePathChanged();
    void accountChanged();
    void wineChanged();
    void versionTextChanged();

private:
    void readWineInfo();

    QString m_uuid;
    QString m_wineVersion;
    ProfileConfig *m_config = nullptr;
    Account *m_account = nullptr;

    QString m_bootVersion;
    QString m_gameVersion;

    QString m_compatibilityToolVersion;
};
