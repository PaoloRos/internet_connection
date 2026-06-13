#pragma once

/**
 * @file ConnectivityChecker.h
 * @brief Connectivity domain model and asynchronous checker service.
 *
 * @mainpage Internet Connection (macOS Qt)
 * @brief Lightweight desktop monitor for internet reachability.
 *
 * This project provides a small Qt Widgets application that reports whether the
 * machine can reach the internet in near real time.
 *
 * Core purpose:
 * - Show a clear binary status: Connected / Not connected.
 * - Keep checks periodic and non-blocking for UI responsiveness.
 * - Expose useful diagnostics (method, latency, error, timestamp) for quick troubleshooting.
 *
 * Architecture summary:
 * - `ConnectivityChecker` executes checks asynchronously.
 * - Primary probe: HTTP `HEAD` toward Apple captive-portal endpoint.
 * - Fallback probe: ICMP ping with timeout.
 * - `MainWindow` consumes normalized status updates and renders the GUI.
 *
 * Credits:
 * - Product idea and requirements: user.
 * - Architecture and implementation: Codex assistant.
 * - AI model used in this environment: Codex (GPT-5 family).
 */

#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QUrl>
#include <optional>

class QNetworkAccessManager;
class QNetworkReply;
class QProcess;

/**
 * @brief Immutable result payload for one connectivity check cycle.
 */
struct ConnectivityStatus {
    /** @brief True when internet reachability is confirmed by HTTP or ping fallback. */
    bool connected = false;
    /** @brief Human-readable name of the method that produced this result. */
    QString method;
    /** @brief End-to-end latency in milliseconds, if measured for this cycle. */
    std::optional<int> latencyMs;
    /** @brief Error detail when not connected or when fallback was needed. */
    QString lastError;
    /** @brief Local timestamp when the cycle finished. */
    QDateTime checkedAt;
};

/**
 * @brief Asynchronous service that performs internet reachability checks.
 *
 * The checker guarantees at most one active cycle at a time and emits a single
 * normalized `ConnectivityStatus` on completion.
 */
class ConnectivityChecker : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Construct the checker service.
     * @param parent QObject parent for ownership/lifetime management.
     */
    explicit ConnectivityChecker(QObject* parent = nullptr);

    /**
     * @brief Start one check cycle if none is currently running.
     *
     * If a check is already in progress, the call is ignored.
     */
    void checkOnce();

signals:
    /**
     * @brief Emitted exactly once per completed check cycle.
     * @param status Normalized connectivity result payload.
     */
    void checkFinished(const ConnectivityStatus& status);

private:
    /** @brief Execute the primary HTTP probe. */
    void runHttpCheck();
    /** @brief Process HTTP probe completion and trigger fallback if needed. */
    void handleHttpFinished(QNetworkReply* reply, qint64 startedMs);
    /** @brief Execute ICMP ping fallback after HTTP failure. */
    void runPingFallback(const QString& cause, qint64 startedMs);
    /** @brief Finalize state and emit `checkFinished`. */
    void finishWithStatus(ConnectivityStatus status);

    /** @brief Qt network manager used for HTTP probes. */
    QNetworkAccessManager* m_networkManager;
    /** @brief Active HTTP reply pointer, null when no HTTP request is active. */
    QPointer<QNetworkReply> m_activeReply;
    /** @brief Active ping process pointer, null when no ping is active. */
    QPointer<QProcess> m_activePing;
    /** @brief True while a check cycle is running. */
    bool m_checkInProgress = false;
    /** @brief HTTP probe timeout in milliseconds. */
    static constexpr int kHttpTimeoutMs = 1200;
    /** @brief Ping fallback timeout in milliseconds. */
    static constexpr int kPingTimeoutMs = 1200;
};
