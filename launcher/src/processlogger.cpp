// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processlogger.h"
#include "umbra_log.h"
#include "utility.h"

#include <QStandardPaths>

ProcessLogger::ProcessLogger(const QDir &dir, const QString &baseName, QProcess *process)
{
    Utility::createPathIfNeeded(dir.absolutePath());

    m_file.setFileName(dir.absoluteFilePath(QStringLiteral("%1.log").arg(baseName)));
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qCWarning(UMBRA_LOG()) << "Failed to open" << m_file.fileName() << "for writing:" << m_file.errorString();
    }

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process] {
        m_file.write(process->readAllStandardOutput());
        m_file.flush();
    });

    connect(process, &QProcess::readyReadStandardError, this, [this, process] {
        m_file.write(process->readAllStandardError());
        m_file.flush();
    });

    connect(process, &QProcess::finished, this, [this] {
        deleteLater();
    });

    qInfo(UMBRA_LOG) << "Client logs are being written to" << m_file.fileName().toUtf8().constData();
}
