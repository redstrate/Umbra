// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"
#include "umbra_http_log.h"
#include "umbra_log.h"

#include <QSslConfiguration>

using namespace Qt::StringLiterals;

void Utility::printRequest(const QString &type, const QNetworkRequest &request)
{
    qDebug(UMBRA_HTTP) << type.toUtf8().constData() << request.url().toDisplayString();
}

void Utility::createPathIfNeeded(const QDir &dir)
{
    if (!QDir().exists(dir.absolutePath())) {
        Q_UNUSED(QDir().mkpath(dir.absolutePath()))
    }
}

void Utility::setSSL(QNetworkRequest &request)
{
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

QString Utility::readVersion(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(UMBRA_LOG) << "Failed to open" << file.fileName() << "for reading:" << file.errorString();
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}

void Utility::writeVersion(const QString &path, const QString &version)
{
    QFile verFile(path);
    if (!verFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(UMBRA_LOG) << "Not able to write" << version << "to" << path << "because" << verFile.errorString();
        return;
    }
    verFile.write(version.toUtf8());
    verFile.close();
}

QString Utility::repositoryFromPatchUrl(const QString &url)
{
    auto url_parts = url.split('/'_L1);
    return url_parts[url_parts.size() - 3];
}
