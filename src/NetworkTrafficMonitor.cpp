#include "NetworkTrafficMonitor.h"

#include <QDateTime>

#include <algorithm>
#include <cmath>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>

namespace {
qint64 nowMs()
{
    return QDateTime::currentMSecsSinceEpoch();
}
}

NetworkTrafficMonitor::NetworkTrafficMonitor(QObject* parent)
    : QObject(parent)
{
}

void NetworkTrafficMonitor::sampleOnce()
{
    NetworkTrafficStatus status;
    status.requiredSampleCount = kStatisticsSampleCount;
    status.sampledAt = QDateTime::currentDateTime();

    const auto counters = readCounters();
    const qint64 sampledMs = nowMs();
    if (!counters.has_value()) {
        emit sampleFinished(status);
        return;
    }

    if (m_previousReceivedBytes.has_value() && m_previousSentBytes.has_value() && m_previousSampleMs.has_value()) {
        const qint64 elapsedMs = sampledMs - *m_previousSampleMs;
        if (elapsedMs > 0
            && counters->receivedBytes >= *m_previousReceivedBytes
            && counters->sentBytes >= *m_previousSentBytes) {
            const double elapsedSeconds = static_cast<double>(elapsedMs) / 1000.0;
            status.downloadBytesPerSecond = static_cast<double>(counters->receivedBytes - *m_previousReceivedBytes) / elapsedSeconds;
            status.uploadBytesPerSecond = static_cast<double>(counters->sentBytes - *m_previousSentBytes) / elapsedSeconds;

            m_downloadSamples.push_back(*status.downloadBytesPerSecond);
            m_uploadSamples.push_back(*status.uploadBytesPerSecond);
            while (static_cast<int>(m_downloadSamples.size()) > kStatisticsSampleCount) {
                m_downloadSamples.pop_front();
            }
            while (static_cast<int>(m_uploadSamples.size()) > kStatisticsSampleCount) {
                m_uploadSamples.pop_front();
            }
        }
    }

    status.sampleCount = static_cast<int>(std::min(m_downloadSamples.size(), m_uploadSamples.size()));
    status.downloadStatistics = statisticsFor(m_downloadSamples);
    status.uploadStatistics = statisticsFor(m_uploadSamples);

    m_previousReceivedBytes = counters->receivedBytes;
    m_previousSentBytes = counters->sentBytes;
    m_previousSampleMs = sampledMs;

    emit sampleFinished(status);
}

std::optional<NetworkTrafficMonitor::Counters> NetworkTrafficMonitor::readCounters() const
{
    ifaddrs* interfaces = nullptr;
    if (getifaddrs(&interfaces) != 0) {
        return std::nullopt;
    }

    Counters counters;
    for (ifaddrs* item = interfaces; item != nullptr; item = item->ifa_next) {
        if (item->ifa_addr == nullptr || item->ifa_data == nullptr) {
            continue;
        }

        if (item->ifa_addr->sa_family != AF_LINK) {
            continue;
        }

        const unsigned int flags = item->ifa_flags;
        const bool usableInterface = (flags & IFF_UP) && !(flags & IFF_LOOPBACK);
        if (!usableInterface) {
            continue;
        }

        const auto* data = static_cast<const if_data*>(item->ifa_data);
        counters.receivedBytes += static_cast<quint64>(data->ifi_ibytes);
        counters.sentBytes += static_cast<quint64>(data->ifi_obytes);
    }

    freeifaddrs(interfaces);
    return counters;
}

TrafficStatistics NetworkTrafficMonitor::statisticsFor(const std::deque<double>& samples) const
{
    TrafficStatistics statistics;
    if (static_cast<int>(samples.size()) < kStatisticsSampleCount) {
        return statistics;
    }

    double sum = 0.0;
    for (const double sample : samples) {
        sum += sample;
    }

    const double mean = sum / static_cast<double>(samples.size());
    double squaredDifferenceSum = 0.0;
    for (const double sample : samples) {
        const double difference = sample - mean;
        squaredDifferenceSum += difference * difference;
    }

    statistics.meanBytesPerSecond = mean;
    statistics.stdDevBytesPerSecond = std::sqrt(squaredDifferenceSum / static_cast<double>(samples.size()));
    return statistics;
}
