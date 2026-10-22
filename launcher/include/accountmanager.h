// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>

#include "account.h"

class AccountManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.accountManager")

public:
    explicit AccountManager(LauncherCore *core, QObject *parent = nullptr);

    void load();

    enum CustomRoles {
        AccountRole = Qt::UserRole,
    };

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Account *createSquareEnixAccount(const QString &username);

    [[nodiscard]] Account *getByUuid(const QString &uuid) const;

    Q_INVOKABLE bool canDelete() const;
    Q_INVOKABLE void deleteAccount(Account *account);

    static QString getDefaultWinePrefixPath(const QString &uuid);

Q_SIGNALS:
    void accountAdded(Account *account);

private:
    void insertAccount(Account *account);

    QList<Account *> m_accounts;
    LauncherCore *m_core;
};
