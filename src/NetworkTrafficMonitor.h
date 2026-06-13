#pragma once

#include <QObject>
#include <QDateTime>
#include <optional>
#include <deque>

struct TrafficStatistics {
    std::optional<double> meanBytesPerSecond;
    std::optional<double> stdDevBytesPerSecond;
};

struct NetworkTrafficStatus {
    std::optional<double> downloadBytesPerSecond;
    std::optional<double> uploadBytesPerSecond;
    TrafficStatistics downloadStatistics;
    TrafficStatistics uploadStatistics;
    int sampleCount = 0;
    int requiredSampleCount = 0;
    QDateTime sampledAt;
};

class NetworkTrafficMonitor : public QObject {
    Q_OBJECT
public:
    explicit NetworkTrafficMonitor(QObject* parent = nullptr);

    void sampleOnce();

signals:
    void sampleFinished(const NetworkTrafficStatus& status);

private:
    struct Counters {
        quint64 receivedBytes = 0;
        quint64 sentBytes = 0;
    };

    std::optional<Counters> readCounters() const;
    TrafficStatistics statisticsFor(const std::deque<double>& samples) const;

    std::optional<quint64> m_previousReceivedBytes;
    std::optional<quint64> m_previousSentBytes;
    std::optional<qint64> m_previousSampleMs;
    std::deque<double> m_downloadSamples;
    std::deque<double> m_uploadSamples;

    static constexpr int kStatisticsSampleCount = 20;
};
