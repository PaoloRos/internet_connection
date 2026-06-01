#include "MainWindow.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_checker(this)
    , m_timer(new QTimer(this))
    , m_indicator(nullptr)
    , m_statusLabel(nullptr)
    , m_methodValue(nullptr)
    , m_latencyValue(nullptr)
    , m_errorValue(nullptr)
    , m_lastCheckValue(nullptr)
{
    buildUi();

    connect(&m_checker, &ConnectivityChecker::checkFinished, this, &MainWindow::onCheckFinished);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::refreshStatus);

    m_timer->start(kRefreshMs);
    refreshStatus();
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("Internet Connection"));
    setMinimumSize(520, 280);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    auto* statusBox = new QGroupBox(QStringLiteral("Connection Status"), central);
    auto* statusLayout = new QHBoxLayout(statusBox);

    m_indicator = new QLabel(statusBox);
    m_indicator->setFixedSize(26, 26);
    m_indicator->setStyleSheet(QStringLiteral("border-radius: 13px; background: #A00000;"));

    m_statusLabel = new QLabel(QStringLiteral("Not connected"), statusBox);
    QFont statusFont = m_statusLabel->font();
    statusFont.setPointSize(16);
    statusFont.setBold(true);
    m_statusLabel->setFont(statusFont);

    statusLayout->addWidget(m_indicator);
    statusLayout->addSpacing(10);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();

    auto* diagBox = new QGroupBox(QStringLiteral("Diagnostics"), central);
    auto* grid = new QGridLayout(diagBox);

    auto addRow = [grid](int row, const QString& name, QLabel*& valueLabel) {
        auto* label = new QLabel(name, grid->parentWidget());
        valueLabel = new QLabel(QStringLiteral("-"), grid->parentWidget());
        valueLabel->setWordWrap(true);
        grid->addWidget(label, row, 0);
        grid->addWidget(valueLabel, row, 1);
    };

    addRow(0, QStringLiteral("Method:"), m_methodValue);
    addRow(1, QStringLiteral("Latency:"), m_latencyValue);
    addRow(2, QStringLiteral("Last error:"), m_errorValue);
    addRow(3, QStringLiteral("Last check:"), m_lastCheckValue);

    grid->setColumnStretch(1, 1);

    root->addWidget(statusBox);
    root->addWidget(diagBox);

    setCentralWidget(central);
}

void MainWindow::refreshStatus()
{
    m_checker.checkOnce();
}

void MainWindow::onCheckFinished(const ConnectivityStatus& status)
{
    updateIndicator(status.connected);
    m_statusLabel->setText(status.connected ? QStringLiteral("Connected") : QStringLiteral("Not connected"));
    m_methodValue->setText(status.method.isEmpty() ? QStringLiteral("-") : status.method);
    m_latencyValue->setText(status.latencyMs.has_value() ? QStringLiteral("%1 ms").arg(*status.latencyMs) : QStringLiteral("-"));
    m_errorValue->setText(status.lastError.isEmpty() ? QStringLiteral("-") : status.lastError);
    m_lastCheckValue->setText(status.checkedAt.isValid()
                                  ? status.checkedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))
                                  : QStringLiteral("-"));
}

void MainWindow::updateIndicator(bool connected)
{
    if (connected) {
        m_indicator->setStyleSheet(QStringLiteral("border-radius: 13px; background: #007F2D;"));
    } else {
        m_indicator->setStyleSheet(QStringLiteral("border-radius: 13px; background: #A00000;"));
    }
}
