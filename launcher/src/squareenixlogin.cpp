// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "squareenixlogin.h"

#include <KFormat>
#include <KLocalizedString>
#include <KSandbox>
#include <QCoroSignal>
#include <QDesktopServices>
#include <QFile>
#include <QNetworkReply>
#include <QRegularExpressionMatch>
#include <QUrlQuery>
#include <QtConcurrentMap>
#include <qcorofuture.h>
#include <qcoronetworkreply.h>

#include "account.h"
#include "accountconfig.h"
#include "umbra_log.h"
#include "launchercore.h"
#include "patcher.h"
#include "physis.hpp"
#include "profileconfig.h"
#include "utility.h"

const QByteArray patchUserAgent = QByteArrayLiteral("FFXIV PATCH CLIENT");
const QByteArray macosPatchUserAgent = QByteArrayLiteral("FFXIV-MAC PATCH CLIEN");

using namespace Qt::StringLiterals;

SquareEnixLogin::SquareEnixLogin(LauncherCore &window, QObject *parent)
    : QObject(parent)
    , m_launcher(window)
{
}

QCoro::Task<std::optional<LoginAuth>> SquareEnixLogin::login(LoginInformation *info)
{
    Q_ASSERT(info != nullptr);
    m_info = info;

    // First, let's check for boot updates. While not technically required for us, it's needed for later hash checking.
    // It's also a good idea anyway, in case the official launcher is needed.
    while (m_lastRunHasPatched) {
        // There seems to be a limitation in their boot patching system.
        // Their server can only give one patch a time, so the boot process must keep trying to patch until
        // there is no patches left.
        if (!co_await checkBootUpdates()) {
            co_return std::nullopt;
        }
    }

    // Login with through the oauth API. This gives us some information like a temporary SID, region and expansion information
    if (!co_await loginOAuth()) {
        co_return std::nullopt;
    }

    // Register the session with the server. This method also updates the game as necessary.
    if (!co_await registerSession()) {
        co_return std::nullopt;
    }

    m_auth.account = info->profile->account();
    co_return m_auth;
}

QCoro::Task<bool> SquareEnixLogin::checkBootUpdates()
{
    m_lastRunHasPatched = false;

    Q_EMIT m_launcher.stageChanged(i18n("Checking for launcher updates..."));
    qInfo(UMBRA_LOG) << "Checking for updates to boot components...";

    QUrl url = QUrl::fromUserInput(m_info->profile->account()->config()->bootPatchServer());
    url.setPath(QStringLiteral("/http/%2/%3").arg(m_info->profile->config()->bootUpdateChannel(), m_info->profile->bootVersion()));

    auto request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, patchUserAgent);

    request.setRawHeader(QByteArrayLiteral("Host"), m_info->profile->account()->config()->bootPatchServer().toUtf8());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = m_launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() == QNetworkReply::NoError) {
        const QString patchList = QString::fromUtf8(reply->readAll());
        if (!patchList.isEmpty()) {
            qDebug(UMBRA_LOG) << "Boot patch list:" << patchList;

            const std::string patchListStd = patchList.toStdString();
            const auto parsedPatchList = physis_parse_patchlist(PatchListType::Boot, patchListStd.c_str());

            if (!m_info->profile->config()->allowPatching()) {
                Q_EMIT m_launcher.loginError(
                    i18n("You require an update to play, but you have the “Allow Updates” option checked - so the login was canceled."));
                co_return false;
            }

            const qint64 neededSpace = parsedPatchList.total_size_downloaded;
            KFormat format;
            QString neededSpaceStr = format.formatByteSize(neededSpace);
            Q_EMIT m_launcher.requiresUpdate(
                i18n("The boot components require an update, which will download %1 of data. Do you still want to continue?", neededSpaceStr));
            const bool wantsToUpdate = co_await qCoro(&m_launcher, &LauncherCore::updateDecided);
            if (!wantsToUpdate) {
                co_return false;
            }

            m_patcher = new Patcher(m_launcher, m_info->profile->config()->gamePath(), true, this);
            const bool hasPatched = co_await m_patcher->patch(parsedPatchList);
            if (hasPatched) {
                // update game version information
                m_info->profile->readGameVersion();
            } else {
                co_return false;
            }
            m_lastRunHasPatched = true;
            m_patcher->deleteLater();
        }
    } else {
        qWarning(UMBRA_LOG) << "Unknown error when verifying boot files:" << reply->errorString();
        Q_EMIT m_launcher.loginError(i18n("Unknown error when verifying boot files.\n\n%1", reply->errorString()));
        co_return false;
    }

    co_return true;
}

QCoro::Task<std::optional<SquareEnixLogin::StoredInfo>> SquareEnixLogin::getStoredValue()
{
    qInfo(UMBRA_LOG) << "Getting the STORED value...";

    Q_EMIT m_launcher.stageChanged(i18n("Logging in..."));

    QUrlQuery query;
    // en is always used to the top url
    query.addQueryItem(QStringLiteral("lng"), QStringLiteral("en"));
    // for some reason, we always use region 3. the actual region is acquired later
    query.addQueryItem(QStringLiteral("rgn"), QString::number(3));
    query.addQueryItem(QStringLiteral("cssmode"), QString::number(1));
    query.addQueryItem(QStringLiteral("isnew"), QString::number(1));
    query.addQueryItem(QStringLiteral("launchver"), QString::number(3));

    QUrl url = QUrl::fromUserInput(m_info->profile->account()->config()->loginServer());
    url.setPath(QStringLiteral("/oauth/ffxiv/login/top"));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    m_launcher.buildRequest(*m_info->profile, request);

    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = m_launcher.mgr()->get(request);
    co_await reply;

    m_username = m_info->username;

    const QString str = QString::fromUtf8(reply->readAll());
    const static QRegularExpression re(QStringLiteral(R"lit(\t<\s*input .* name="_STORED_" value="(?<stored>.*)">)lit"));
    const QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        co_return StoredInfo{match.captured(1), url};
    } else {
        Q_EMIT m_launcher.loginError(
            i18n("Square Enix servers refused to confirm session information. The game may be under maintenance, try the official launcher."));
        co_return {};
    }
}

QCoro::Task<bool> SquareEnixLogin::loginOAuth()
{
    const auto storedResult = co_await getStoredValue();
    if (storedResult == std::nullopt) {
        co_return false;
    }

    const auto [stored, referer] = *storedResult;

    qInfo(UMBRA_LOG) << "Logging in...";

    QUrlQuery postData;
    postData.addQueryItem(QStringLiteral("_STORED_"), stored);
    postData.addQueryItem(QStringLiteral("sqexid"), m_info->username);
    postData.addQueryItem(QStringLiteral("password"), m_info->password);

    QUrl url = QUrl::fromUserInput(m_info->profile->account()->config()->loginServer());
    url.setPath(QStringLiteral("/oauth/ffxiv/login/login.send"));

    QNetworkRequest request(url);
    m_launcher.buildRequest(*m_info->profile, request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader(QByteArrayLiteral("Referer"), referer.toEncoded());
    request.setRawHeader(QByteArrayLiteral("Cache-Control"), QByteArrayLiteral("no-cache"));

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = m_launcher.mgr()->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    m_launcher.setupIgnoreSSL(reply);
    co_await reply;

    const QString str = QString::fromUtf8(reply->readAll());

    const static QRegularExpression re(QStringLiteral(R"lit(window.external.user\("login=auth,ok,(?<launchParams>.*)\);)lit"));
    const QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        const auto parts = match.captured(1).split(','_L1);

        const bool terms = parts[3] == "1"_L1;
        const bool playable = parts[9] == "1"_L1;

        if (!playable) {
            Q_EMIT m_launcher.loginError(i18n("Your account is unplayable. Check that you have the correct license, and a valid subscription."));
            co_return false;
        }

        if (!terms) {
            Q_EMIT m_launcher.loginError(i18n("Your account is unplayable. You need to accept the terms of service from the official launcher first."));
            co_return false;
        }

        m_SID = parts[1];
        m_auth.region = parts[5].toInt();

        co_return true;
    } else {
        const static QRegularExpression errorRe(QStringLiteral(R"lit(window.external.user\("login=auth,ng,err,(?<launchParams>.*)\);)lit"));
        const QRegularExpressionMatch errorMatch = errorRe.match(str);

        if (errorMatch.hasCaptured(1)) {
            // there's a stray quote at the end of the error string, so let's remove that
            Q_EMIT m_launcher.loginError(errorMatch.captured(1).chopped(1));
        } else {
            Q_EMIT m_launcher.loginError(i18n("Unknown error"));
        }

        co_return false;
    }
}

QCoro::Task<bool> SquareEnixLogin::registerSession()
{
    qInfo(UMBRA_LOG) << "Registering the session...";

    QUrl url = QUrl::fromUserInput(m_info->profile->account()->config()->gamePatchServer());
    url.setPath(QStringLiteral("/http/%2/%3/%4").arg(m_info->profile->config()->gameUpdateChannel(), m_info->profile->baseGameVersion(), m_SID));

    auto request = QNetworkRequest(url);
    Utility::setSSL(request);
    request.setRawHeader(QByteArrayLiteral("X-Hash-Check"), QByteArrayLiteral("enabled"));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, patchUserAgent);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = m_launcher.mgr()->post(request, QByteArray{});
    co_await reply;

    if (reply->error() == QNetworkReply::NoError) {
        QString patchUniqueId;
        if (reply->rawHeaderList().contains(QByteArrayLiteral("X-Patch-Unique-Id"))) {
            patchUniqueId = QString::fromUtf8(reply->rawHeader(QByteArrayLiteral("X-Patch-Unique-Id")));
        } else if (reply->rawHeaderList().contains(QByteArrayLiteral("x-patch-unique-id"))) {
            patchUniqueId = QString::fromUtf8(reply->rawHeader(QByteArrayLiteral("x-patch-unique-id")));
        }

        if (!patchUniqueId.isEmpty()) {
            const QString body = QString::fromUtf8(reply->readAll());

            if (!body.isEmpty()) {
                qDebug(UMBRA_LOG) << "Game patch list:" << body;

                if (!m_info->profile->config()->allowPatching()) {
                    Q_EMIT m_launcher.loginError(
                        i18n("You require an update to play, but you have the “Allow Updates” option checked - so the login was canceled."));
                    co_return false;
                }

                std::string bodyStd = body.toStdString();
                const auto parsedPatchList = physis_parse_patchlist(PatchListType::Game, bodyStd.c_str());

                const qint64 neededSpace = parsedPatchList.total_size_downloaded;
                KFormat format;
                QString neededSpaceStr = format.formatByteSize(neededSpace);
                Q_EMIT m_launcher.requiresUpdate(
                    i18n("The game require an update, which will download %1 of data. Do you still want to continue?", neededSpaceStr));
                const bool wantsToUpdate = co_await qCoro(&m_launcher, &LauncherCore::updateDecided);
                if (!wantsToUpdate) {
                    co_return false;
                }

                m_patcher = new Patcher(m_launcher, m_info->profile->config()->gamePath(), false, this);
                const bool hasPatched = co_await m_patcher->patch(parsedPatchList);
                m_patcher->deleteLater();
                if (!hasPatched) {
                    co_return false;
                }

                // re-read game version if it has updated
                m_info->profile->readGameVersion();
            }

            m_auth.SID = patchUniqueId;

            co_return true;
        } else {
            Q_EMIT m_launcher.loginError(i18n("Fatal error, request was successful but X-Patch-Unique-Id was not recieved."));
        }
    } else {
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            Q_EMIT m_launcher.loginError(
                i18n("SSL handshake error detected. If you are using OpenSUSE or Fedora, try running `update-crypto-policies --set LEGACY`."));
        } else if (reply->error() == QNetworkReply::ContentConflictError) {
            Q_EMIT m_launcher.loginError(i18n("The boot files are outdated, please login in again to update them."));
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 405) {
            Q_EMIT m_launcher.loginError(i18n("The game failed the anti-tamper check. Restore the game to the original state and try updating again."));
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 410) {
            Q_EMIT m_launcher.loginError(i18n("This game version is no longer supported."));
        } else {
            Q_EMIT m_launcher.loginError(i18n("Unknown error when registering the session."));
        }
    }

    co_return false;
}

#include "moc_squareenixlogin.cpp"
