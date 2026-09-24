pragma Singleton

import QtQuick

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------
// Central place for colors, spacing and font sizes. Using a singleton means a
// later release can add dark/light switching without touching any page.
//
// The palette follows the Google Photos look that the D version targets:
// white surfaces, a single blue accent, generous white space.
// ---------------------------------------------------------------------------
QtObject {
    id: theme

    // ---- color -------------------------------------------------------------
    readonly property color background: "#FFFFFF"
    readonly property color surface: "#FFFFFF"
    readonly property color surfaceVariant: "#F1F3F4"
    readonly property color outline: "#DADCE0"

    readonly property color primary: "#1A73E8"

    // Text and icons drawn on top of `primary`.
    //
    // This was called onPrimary, which QML rejects:
    //     Theme.qml:24: Cannot assign a value to a signal
    //     (expecting a script to be run)
    // In QML any identifier matching  on + UpperCase  is parsed as a signal
    // handler, so `onPrimary:` reads as "assign to the Primary signal". The
    // whole Theme singleton failed to load, which cascaded:
    //     Theme unavailable -> AlbumCard unavailable
    //                       -> AlbumsPage unavailable
    //                       -> the root component never loaded, blank screen.
    // The name avoids the `on` prefix for that reason.
    readonly property color primaryText: "#FFFFFF"

    readonly property color textPrimary: "#202124"
    readonly property color textSecondary: "#5F6368"
    readonly property color textDisabled: "#9AA0A6"

    readonly property color selectedPill: "#E8F0FE"
    readonly property color scrim: "#66000000"

    // ---- spacing -----------------------------------------------------------
    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 16
    readonly property int spacingLg: 24
    readonly property int spacingXl: 32

    // ---- radii -------------------------------------------------------------
    readonly property int radiusSm: 4
    readonly property int radiusMd: 8
    readonly property int radiusLg: 16

    // ---- typography --------------------------------------------------------
    readonly property int fontTitle: 22
    readonly property int fontHeading: 16
    readonly property int fontBody: 14
    readonly property int fontCaption: 12

    // ---- metrics -----------------------------------------------------------
    readonly property int bottomBarHeight: 56
    readonly property int topBarHeight: 56
    readonly property int gridThumbSize: 110
    readonly property int gridSpacing: 2
}
