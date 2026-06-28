// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QNetworkRequest>

namespace Utility
{
void printRequest(const QString &type, const QNetworkRequest &request);
void createPathIfNeeded(const QDir &dir);
void setSSL(QNetworkRequest &request);
QString readVersion(const QString &path);
void writeVersion(const QString &path, const QString &version);
QString repositoryFromPatchUrl(const QString &url);
QString toWindowsPath(const QDir &dir);
}
