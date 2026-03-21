// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptedarg.h"
#include "blowfish.h"

#include <QDebug>
#include <physis.hpp>

#if defined(Q_OS_MAC)
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#if defined(Q_OS_MAC)
// taken from XIV-on-Mac, apparently Wine changed this?
uint32_t TickCount()
{
    struct mach_timebase_info timebase;
    mach_timebase_info(&timebase);

    auto machtime = mach_continuous_time();
    auto numer = uint64_t(timebase.numer);
    auto denom = uint64_t(timebase.denom);
    auto monotonic_time = machtime * numer / denom / 100;
    return monotonic_time / 10000;
}
#endif

#if defined(Q_OS_LINUX)
uint32_t TickCount()
{
    struct timespec ts {
    };

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if defined(Q_OS_WIN)
uint32_t TickCount()
{
    return GetTickCount();
}
#endif

QString encryptGameArg(const QString &arg)
{
    const uint32_t ticks = TickCount();

    char buffer[9]{};
    sprintf(buffer, "%08x", ticks & ~0xFFFF);

    QByteArray toEncrypt = (QStringLiteral(" T =%1").arg(ticks) + arg).toLatin1();
    size_t oldSize = toEncrypt.size();
    toEncrypt.resize(1024, 0);

    InitializeBlowfish(buffer, 8);

    size_t commandLineSize = oldSize + 1;
    for(unsigned int i = 0; i < (commandLineSize & ~0x7); i += 8)
    {
        Blowfish_encipher(
            reinterpret_cast<uint32_t*>(toEncrypt.data() + i),
            reinterpret_cast<uint32_t*>(toEncrypt.data() + i + 4));
    }

    toEncrypt.resize(commandLineSize);

    const QString base64 = QString::fromUtf8(toEncrypt.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::KeepTrailingEquals));

    return QStringLiteral("sqex0002%1!////").arg(base64);
}
