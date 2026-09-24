#pragma once

#include <QStringList>

// ---------------------------------------------------------------------------
// DatabaseSchema
// ---------------------------------------------------------------------------
// Single source of truth for the SQLite schema used by FenSu Cloud Album D.
//
// The schema is intentionally created in one place so future migrations can
// be appended here without touching the rest of the code base.
//
// Rules that the schema obeys:
//   - every byte-size column is INTEGER (64 bit), never 32 bit types
//   - every timestamp is stored as a Unix epoch in **milliseconds** (INTEGER)
//   - the album tree mirrors the WebDAV directory structure exactly
//   - location fields are pre-created for a future release but unused in D
// ---------------------------------------------------------------------------
namespace DatabaseSchema
{
    // Schema version, bump together with a migration entry.
    constexpr int kCurrentVersion = 1;

    // Returns the list of "CREATE TABLE IF NOT EXISTS" statements that make
    // up version 1 of the schema, in dependency order.
    QStringList creationStatements();

    // Returns statements that must run after creation to make an existing
    // database consistent with the current version (indexes, etc.).
    QStringList indexStatements();

    // Name of the settings table, used by DatabaseManager helpers.
    constexpr auto kSettingsTable = "settings";
}