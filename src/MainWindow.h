#pragma once

#include "ConnectivityChecker.h"

#include <QMainWindow>

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

private:
    /** @brief Build and wire all visible UI elements. */
    void buildUi();
    /** @brief Update the status indicator color for connected/disconnected states. */
    void updateIndicator(bool connected);

    /** @brief Connectivity service handling asynchronous probes. */
    ConnectivityChecker m_checker;
    /** @brief Periodic trigger timer for check scheduling. */
    QTimer* m_timer;

    /** @brief Colored circular status indicator (green/red). */
    QLabel* m_indicator;
    /** @brief Main status text label. */
    QLabel* m_statusLabel;
    /** @brief Diagnostics: method used for last check. */
    QLabel* m_methodValue;
    /** @brief Diagnostics: measured latency for last check. */
    QLabel* m_latencyValue;
    /** @brief Diagnostics: last error details, if any. */
    QLabel* m_errorValue;
    /** @brief Diagnostics: timestamp of last completed check. */
    QLabel* m_lastCheckValue;

    /** @brief Refresh period in milliseconds. */
    static constexpr int kRefreshMs = 500;
};
