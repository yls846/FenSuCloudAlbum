#include "core/network/NetworkMonitor.h"

#include <QNetworkInformation>
#include <QDebug>

NetworkMonitor::NetworkMonitor(QObject *parent)
    : QObject(parent)
{
}

NetworkMonitor::~NetworkMonitor() = default;

void NetworkMonitor::start()
{
    // QNetworkInformation::loadDefaultBackend() fails on platforms without a
    // usable backend; that must never crash or block the app, so we degrade to
    // "assume online" and let the sync layer surface real errors.
    if (!QNetworkInformation::loadDefaultBackend()) {
        qWarning() << "[NetworkMonitor] no network information backend; assuming online.";
        setOnline(true);
        setBackendName(QStringLiteral("none"));
        return;
    }

    m_information = QNetworkInformation::instance();
    if (!m_information) {
        setOnline(true);
        setBackendName(QStringLiteral("none"));
        return;
    }

    setBackendName(m_information->backendName());

    connect(m_information, &QNetworkInformation::reachabilityChanged,
            this, &NetworkMonitor::handleReachabilityChanged);

    handleReachabilityChanged();
}

void NetworkMonitor::handleReachabilityChanged()
{
    if (!m_information) {
        setOnline(true);
        return;
    }

    const QNetworkInformation::Reachability reachability = m_information->reachability();
    const bool online = reachability == QNetworkInformation::Reachability::Online
                        || reachability == QNetworkInformation::Reachability::Unknown;

    setOnline(online);
}

void NetworkMonitor::setOnline(bool online)
{
    if (m_online == online)
        return;
    m_online = online;
    emit onlineChanged();
}

void NetworkMonitor::setBackendName(const QString &name)
{
    if (m_backendName == name)
        return;
    m_backendName = name;
    emit backendNameChanged();
}