#pragma once

#include <QAbstractListModel>
#include <QDate>
#include <QVector>

class DatabaseManager;

// ---------------------------------------------------------------------------
// TimelineModel
//
// Groups media by *display* date (not original date) into day sections.
// The D version keeps the grouping logic here instead of QML so that a future
// release can switch to a sectioned/streamed implementation without touching
// the UI layer.
// ---------------------------------------------------------------------------
class TimelineModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        DateRole = Qt::UserRole + 1,
        TitleRole,
        SubtitleRole,
        ItemCountRole,
        CoverPathRole,
        IsTodayRole,
    };
    Q_ENUM(Roles)

    struct DaySection {
        QDate date;
        QString title;
        QString coverPath;
        qint64 itemCount = 0;
        bool isToday = false;
    };

    explicit TimelineModel(QObject *parent = nullptr);

    void setDatabase(DatabaseManager *database);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void clear();

    int count() const { return m_sections.size(); }

signals:
    void countChanged();

private:
    QVector<DaySection> m_sections;
    DatabaseManager *m_database = nullptr;
};