#pragma once

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QtQml/qqmlregistration.h>

// ---------------------------------------------------------------------------
// MediaItem
//
// Plain value type describing a single media row (photo or video).
//
// Hard requirements honoured here (see task book chapters 10 / 17 / 18):
//   * fileSize is qint64 -> supports single files of 5 GB and beyond.
//     NEVER downgrade this to int / quint32.
//   * durationMs is qint64 as well (long videos).
//   * originalDateTaken keeps the immutable EXIF/creation timestamp,
//     displayDateTaken is what the UI sorts by and the user may override.
//   * location fields exist as reserved slots for a future release;
//     the D-version UI must NOT display them.
// ---------------------------------------------------------------------------
struct MediaItem
{
    Q_GADGET

    Q_PROPERTY(qint64 id MEMBER id)
    Q_PROPERTY(qint64 albumId MEMBER albumId)
    Q_PROPERTY(QString fileName MEMBER fileName)
    Q_PROPERTY(QString displayName MEMBER displayName)
    Q_PROPERTY(QString relativePath MEMBER relativePath)
    Q_PROPERTY(QString mimeType MEMBER mimeType)
    Q_PROPERTY(qint64 fileSize MEMBER fileSize)
    Q_PROPERTY(qint64 width MEMBER width)
    Q_PROPERTY(qint64 height MEMBER height)
    Q_PROPERTY(qint64 durationMs MEMBER durationMs)
    Q_PROPERTY(QDateTime originalDateTaken MEMBER originalDateTaken)
    Q_PROPERTY(QDateTime displayDateTaken MEMBER displayDateTaken)
    Q_PROPERTY(bool favorite MEMBER favorite)
    Q_PROPERTY(bool isVideo MEMBER isVideo)
    Q_PROPERTY(QString thumbnailPath MEMBER thumbnailPath)

public:
    qint64 id = -1;
    qint64 albumId = -1;

    QString fileName;
    QString displayName;

    // Path relative to the WebDAV photos root. The directory structure is
    // authoritative on the server and must never be flattened.
    // Example: "Summer 2024/Trip/Day 3/IMG_0001.jpg"
    QString relativePath;

    QString mimeType;

    // 64-bit on purpose. 5 GB single files are an explicit requirement.
    qint64 fileSize = 0;

    qint64 width = 0;
    qint64 height = 0;
    qint64 durationMs = 0; // 0 for still images

    QDateTime originalDateTaken; // immutable, from EXIF or file mtime
    QDateTime displayDateTaken;  // sortable / user-overridable

    bool favorite = false;
    bool isVideo = false;

    QString thumbnailPath; // local cache path, may be empty

    // ---- reserved for later releases (NOT shown in D version UI) ----
    double latitude = 0.0;
    double longitude = 0.0;
    QString locationName;
    QString locationSource;

    bool isValid() const { return id >= 0 && !fileName.isEmpty(); }
};

Q_DECLARE_METATYPE(MediaItem)
