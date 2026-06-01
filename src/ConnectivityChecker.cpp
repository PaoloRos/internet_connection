#include "ConnectivityChecker.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QTimer>

namespace {
const QUrl kAppleProbeUrl(QStringLiteral("https://captive.apple.com"));

qint64 nowMs()
{
    return QDateTime::currentMSecsSinceEpoch();
}
}

ConnectivityChecker::ConnectivityChecker(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void ConnectivityChecker::checkOnce()
{
    if (m_checkInProgress) {
        return;
    }
    m_checkInProgress = true;
    runHttpCheck();
}

void ConnectivityChecker::runHttpCheck()
{
    const qint64 startedMs = nowMs();

    QNetworkRequest request(kAppleProbeUrl);
    request.setTransferTimeout(kHttpTimeoutMs);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_activeReply = m_networkManager->head(request);

    auto* timeout = new QTimer(m_activeReply);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, this, [this]() {
        if (m_activeReply) {
            m_activeReply->abort();
        }
    });
    timeout->start(kHttpTimeoutMs);

    QObject::connect(m_activeReply, &QNetworkReply::finished, this, [this, startedMs]() {
        if (!m_activeReply) {
            return;
        }
        QNetworkReply* reply = m_activeReply.data();
        m_activeReply = nullptr;
        handleHttpFinished(reply, startedMs);
        reply->deleteLater();
    });
}

void ConnectivityChecker::handleHttpFinished(QNetworkReply* reply, qint64 startedMs)
{
    const auto err = reply->error();
    if (err == QNetworkReply::NoError) {
        ConnectivityStatus status;
        status.connected = true;
        status.method = QStringLiteral("HTTP HEAD (Apple probe)");
        status.latencyMs = static_cast<int>(nowMs() - startedMs);
        status.checkedAt = QDateTime::currentDateTime();
        finishWithStatus(std::move(status));
        return;
    }

    const QString reason = reply->errorString();
    runPingFallback(reason, startedMs);
}

void ConnectivityChecker::runPingFallback(const QString& cause, qint64 startedMs)
{
    m_activePing = new QProcess(this);
    m_activePing->setProgram(QStringLiteral("/sbin/ping"));
    m_activePing->setArguments({QStringLiteral("-c"), QStringLiteral("1"), QStringLiteral("-W"), QStringLiteral("1000"), QStringLiteral("1.1.1.1")});

    auto* timeout = new QTimer(m_activePing);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, this, [this]() {
        if (m_activePing) {
            m_activePing->kill();
        }
    });
    timeout->start(kPingTimeoutMs);

    QObject::connect(m_activePing, &QProcess::finished, this, [this, cause, startedMs](int exitCode, QProcess::ExitStatus exitStatus) {
        QString stdOut;
        QString stdErr;
        if (m_activePing) {
            stdOut = QString::fromUtf8(m_activePing->readAllStandardOutput());
            stdErr = QString::fromUtf8(m_activePing->readAllStandardError());
            m_activePing->deleteLater();
        }
        m_activePing = nullptr;

        ConnectivityStatus status;
        status.checkedAt = QDateTime::currentDateTime();

        const bool pingOk = (exitStatus == QProcess::NormalExit && exitCode == 0);
        if (pingOk) {
            status.connected = true;
            status.method = QStringLiteral("Ping fallback");
            status.latencyMs = static_cast<int>(nowMs() - startedMs);
            finishWithStatus(std::move(status));
            return;
        }

        status.connected = false;
        status.method = QStringLiteral("HTTP + Ping failed");
        status.latencyMs = std::nullopt;

        QStringList errors;
        errors << QStringLiteral("HTTP: %1").arg(cause);
        if (!stdErr.trimmed().isEmpty()) {
            errors << QStringLiteral("Ping stderr: %1").arg(stdErr.trimmed());
        } else if (!stdOut.trimmed().isEmpty()) {
            errors << QStringLiteral("Ping output: %1").arg(stdOut.trimmed());
        } else {
            errors << QStringLiteral("Ping failed (exit code %1)").arg(exitCode);
        }
        status.lastError = errors.join(QStringLiteral(" | "));
        finishWithStatus(std::move(status));
    });

    m_activePing->start();
}

void ConnectivityChecker::finishWithStatus(ConnectivityStatus status)
{
    m_checkInProgress = false;
    emit checkFinished(status);
}
