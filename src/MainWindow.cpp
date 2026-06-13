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
    , m_trafficMonitor(this)
    , m_timer(new QTimer(this))
    , m_trafficTimer(new QTimer(this))
    , m_indicator(nullptr)
    , m_statusLabel(nullptr)
    , m_methodValue(nullptr)
    , m_latencyValue(nullptr)
    , m_downloadCurrentValue(nullptr)
    , m_downloadMeanValue(nullptr)
    , m_downloadStdDevValue(nullptr)
    , m_uploadCurrentValue(nullptr)
    , m_uploadMeanValue(nullptr)
    , m_uploadStdDevValue(nullptr)
    , m_errorValue(nullptr)
    , m_lastCheckValue(nullptr)
{
    buildUi();

    connect(&m_checker, &ConnectivityChecker::checkFinished, this, &MainWindow::onCheckFinished);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::refreshStatus);
    connect(&m_trafficMonitor, &NetworkTrafficMonitor::sampleFinished, this, &MainWindow::onTrafficSampleFinished);
    connect(m_trafficTimer, &QTimer::timeout, &m_trafficMonitor, &NetworkTrafficMonitor::sampleOnce);

    m_timer->start(kRefreshMs);
    m_trafficMonitor.sampleOnce();
    m_trafficTimer->start(kTrafficSampleMs);
    refreshStatus();
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("Internet Connection"));
    setMinimumSize(720, 380);

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

    auto addFullRow = [grid](int row, const QString& name, QLabel*& valueLabel) {
        auto* label = new QLabel(name, grid->parentWidget());
        valueLabel = new QLabel(QStringLiteral("-"), grid->parentWidget());
        valueLabel->setWordWrap(true);
        grid->addWidget(label, row, 0);
        grid->addWidget(valueLabel, row, 1, 1, 3);
    };

    auto addTrafficRow = [grid](int row, const QString& name, QLabel*& currentLabel, QLabel*& meanLabel, QLabel*& stdDevLabel) {
        auto* label = new QLabel(name, grid->parentWidget());
        currentLabel = new QLabel(QStringLiteral("-"), grid->parentWidget());
        meanLabel = new QLabel(QStringLiteral("-"), grid->parentWidget());
        stdDevLabel = new QLabel(QStringLiteral("-"), grid->parentWidget());
        grid->addWidget(label, row, 0);
        grid->addWidget(currentLabel, row, 1);
        grid->addWidget(meanLabel, row, 2);
        grid->addWidget(stdDevLabel, row, 3);
    };

    auto* currentHeader = new QLabel(QStringLiteral("Current"), diagBox);
    auto* meanHeader = new QLabel(QStringLiteral("Mean"), diagBox);
    auto* stdDevHeader = new QLabel(QStringLiteral("Std. dev."), diagBox);
    QFont headerFont = currentHeader->font();
    headerFont.setBold(true);
    currentHeader->setFont(headerFont);
    meanHeader->setFont(headerFont);
    stdDevHeader->setFont(headerFont);
    grid->addWidget(currentHeader, 0, 1);
    grid->addWidget(meanHeader, 0, 2);
    grid->addWidget(stdDevHeader, 0, 3);

    addFullRow(1, QStringLiteral("Method:"), m_methodValue);
    addFullRow(2, QStringLiteral("Latency:"), m_latencyValue);
    addTrafficRow(3, QStringLiteral("Download:"), m_downloadCurrentValue, m_downloadMeanValue, m_downloadStdDevValue);
    addTrafficRow(4, QStringLiteral("Upload:"), m_uploadCurrentValue, m_uploadMeanValue, m_uploadStdDevValue);
    addFullRow(5, QStringLiteral("Last error:"), m_errorValue);
    addFullRow(6, QStringLiteral("Last check:"), m_lastCheckValue);

    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 1);
    grid->setColumnStretch(3, 1);

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

void MainWindow::onTrafficSampleFinished(const NetworkTrafficStatus& status)
{
    m_downloadCurrentValue->setText(formatTransferRate(status.downloadBytesPerSecond));
    m_downloadMeanValue->setText(formatTransferRate(status.downloadStatistics.meanBytesPerSecond));
    m_downloadStdDevValue->setText(formatTransferRate(status.downloadStatistics.stdDevBytesPerSecond));
    m_uploadCurrentValue->setText(formatTransferRate(status.uploadBytesPerSecond));
    m_uploadMeanValue->setText(formatTransferRate(status.uploadStatistics.meanBytesPerSecond));
    m_uploadStdDevValue->setText(formatTransferRate(status.uploadStatistics.stdDevBytesPerSecond));
}

void MainWindow::updateIndicator(bool connected)
{
    if (connected) {
        m_indicator->setStyleSheet(QStringLiteral("border-radius: 13px; background: #007F2D;"));
    } else {
        m_indicator->setStyleSheet(QStringLiteral("border-radius: 13px; background: #A00000;"));
    }
}

QString MainWindow::formatTransferRate(const std::optional<double>& bytesPerSecond) const
{
    if (!bytesPerSecond.has_value()) {
        return QStringLiteral("-");
    }

    double value = *bytesPerSecond;
    QString unit = QStringLiteral("B/s");
    if (value >= 1024.0 * 1024.0) {
        value /= 1024.0 * 1024.0;
        unit = QStringLiteral("MiB/s");
    } else if (value >= 1024.0) {
        value /= 1024.0;
        unit = QStringLiteral("KiB/s");
    }

    return QStringLiteral("%1 %2").arg(value, 0, 'f', value >= 10.0 ? 1 : 2).arg(unit);
}
