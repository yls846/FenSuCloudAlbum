#pragma once

#include <QObject>

class QNetworkInformation;

// ---------------------------------------------------------------------------
// NetworkMonitor
//
// Thin wrapper over QNetworkInformation (Qt 6) exposing a single boolean the
// QML layer can bind to. The D version only needs "are we online", but the
// same object will later feed the sync scheduler with metered/unmetered state.
// ---------------------------------------------------------------------------
class NetworkMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool online READ isOnline NOTIFY onlineChanged)
    Q_PROPERTY(QString backendName READ backendName NOTIFY backendNameChanged)

public:
    explicit NetworkMonitor(QObject *parent = nullptr);
    ~NetworkMonitor() override;

    bool isOnline() const { return m_online; }
    QString backendName() const { return m_backendName; }

    // Called by AppController during startup so failures stay non fatal:
    // if no backend is available we optimistically report "online".
    void start();

signals:
    void onlineChanged();
    void backendNameChanged();

private slots:
    void handleReachabilityChanged();

private:
    void setOnline(bool online);
    void setBackendName(const QString &name);

    QNetworkInformation *m_information = nullptr;
    bool m_online = true;
    QString m_backendName;
};