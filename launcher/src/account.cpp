// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "account.h"

#include <qcorocore.h>
#include <qt6keychain/keychain.h>

#include "accountconfig.h"
#include "umbra_log.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Account::Account(const QString &key, QObject *parent)
    : QObject(parent)
    , m_config(new AccountConfig(key, this))
    , m_key(key)
{
    fetchPassword();

    connect(m_config, &AccountConfig::WinePrefixPathChanged, this, &Account::winePrefixChanged);
}

Account::~Account()
{
    m_config->save();
}

bool Account::isWinePrefixDefault() const
{
    return m_config->winePrefixPath() == m_config->defaultWinePrefixPathValue();
}

QString Account::uuid() const
{
    return m_key;
}

QString Account::getPassword()
{
    return QCoro::waitFor(getKeychainValue(QStringLiteral("password")));
}

void Account::setPassword(const QString &password)
{
    setKeychainValue(QStringLiteral("password"), password);

    if (m_needsPassword) {
        m_needsPassword = false;
        Q_EMIT needsPasswordChanged();
    }
}

void Account::setKeychainValue(const QString &key, const QString &value)
{
    auto job = new QKeychain::WritePasswordJob(QStringLiteral("Umbra"), this);
    job->setTextData(value);
#ifdef FLATPAK
    job->setKey(QStringLiteral("flatpak-") + m_key + QStringLiteral("-") + key);
#else
    job->setKey(m_key + QStringLiteral("-") + key);
#endif
    job->start();

    connect(job, &QKeychain::WritePasswordJob::finished, this, [job] {
        if (job->error() != QKeychain::NoError) {
            qWarning(UMBRA_LOG) << "Error when writing" << job->errorString();
        }
    });
}

QCoro::Task<QString> Account::getKeychainValue(const QString &key)
{
    auto job = new QKeychain::ReadPasswordJob(QStringLiteral("Umbra"), this);
#ifdef FLATPAK
    job->setKey(QStringLiteral("flatpak-") + m_key + QStringLiteral("-") + key);
#else
    job->setKey(m_key + QStringLiteral("-") + key);
#endif
    job->start();

    co_await qCoro(job, &QKeychain::ReadPasswordJob::finished);

    if (job->error() != QKeychain::NoError) {
        qWarning(UMBRA_LOG) << "Error when reading" << key << job->errorString() << "for account" << uuid();
    }

    co_return job->textData();
}

bool Account::needsPassword() const
{
    return m_needsPassword;
}

QCoro::Task<> Account::fetchPassword()
{
    const QString password = co_await getKeychainValue(QStringLiteral("password"));
    m_needsPassword = password.isEmpty();
    Q_EMIT needsPasswordChanged();

    co_return;
}

AccountConfig *Account::config() const
{
    return m_config;
}

#include "moc_account.cpp"
