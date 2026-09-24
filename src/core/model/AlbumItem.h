#pragma once

#include <QString>
#include <QDateTime>

// ---------------------------------------------------------------------------
// AlbumItem
//
// Mirror of one WebDAV directory that represents one album.
// Chapter 10 of the task book: a WebDAV folder maps 1:1 to an album and the
// folder hierarchy must NEVER be flattened.
// ---------------------------------------------------------------------------
struct AlbumItem
{
    qint64 id = -1;

    QString name;

    // Same value as MediaItem::relativePath semantics, but for the folder:
    // an empty string means "the photos root itself".
    QString relativePath;

    qint64 coverMediaId = -1;
    qint64 mediaCount = 0;

    QDateTime createdAt;
    QDateTime updatedAt;

    bool isValid() const { return id >= 0; }
};