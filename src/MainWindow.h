#pragma once

#include "ConnectivityChecker.h"
#include "NetworkTrafficMonitor.h"

#include <QMainWindow>
#include <optional>

class QLabel;
class QTimer;

/**
 * @brief Main GUI controller for visualizing connectivity state and diagnostics.
 *
 * Responsibilities:
 * - Trigger periodic checks at fixed cadence.
 * - Translate `ConnectivityStatus` into user-facing UI state.
 * - Keep the interface minimal: status indicator + concise diagnostics.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Construct and initialize the main window and refresh timer.
     * @param parent Parent widget.
     */
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    /** @brief Request a new connectivity check cycle. */
    void refreshStatus();
    /** @brief Consume checker results and update the GUI. */
    void onCheckFinished(const ConnectivityStatus& status);
    /** @brief Consume traffic samples and update throughput values. */
    void onTrafficSampleFinished(const NetworkTrafficStatus& status);

private:
    /** @brief Build and wire all visible UI elements. */
    void buildUi();
    /** @brief Update the status indicator color for connected/disconnected states. */
    void updateIndicator(bool connected);
    /** @brief Format a byte-per-second rate using compact binary units. */
    QString formatTransferRate(const std::optional<double>& bytesPerSecond) const;

    /** @brief Connectivity service handling asynchronous probes. */
    ConnectivityChecker m_checker;
    /** @brief Traffic service sampling network interface counters. */
    NetworkTrafficMonitor m_trafficMonitor;
    /** @brief Periodic trigger timer for connectivity check scheduling. */
    QTimer* m_timer;
    /** @brief Periodic trigger timer for traffic sampling. */
    QTimer* m_trafficTimer;

    /** @brief Colored circular status indicator (green/red). */
    QLabel* m_indicator;
    /** @brief Main status text label. */
    QLabel* m_statusLabel;
    /** @brief Diagnostics: method used for last check. */
    QLabel* m_methodValue;
    /** @brief Diagnostics: measured latency for last check. */
    QLabel* m_latencyValue;
    /** @brief Diagnostics: current observed download rate. */
    QLabel* m_downloadCurrentValue;
    /** @brief Diagnostics: rolling mean download rate. */
    QLabel* m_downloadMeanValue;
    /** @brief Diagnostics: rolling standard deviation download rate. */
    QLabel* m_downloadStdDevValue;
    /** @brief Diagnostics: current observed upload rate. */
    QLabel* m_uploadCurrentValue;
    /** @brief Diagnostics: rolling mean upload rate. */
    QLabel* m_uploadMeanValue;
    /** @brief Diagnostics: rolling standard deviation upload rate. */
    QLabel* m_uploadStdDevValue;
    /** @brief Diagnostics: last error details, if any. */
    QLabel* m_errorValue;
    /** @brief Diagnostics: timestamp of last completed check. */
    QLabel* m_lastCheckValue;

    /** @brief Refresh period in milliseconds. */
    static constexpr int kRefreshMs = 500;
    /** @brief Throughput sampling period in milliseconds. */
    static constexpr int kTrafficSampleMs = 1000;
};
