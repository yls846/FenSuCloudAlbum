#include "core/model/MediaItem.h"

// MediaItem is a header-only value type. This translation unit exists so that
// the class participates in the build (and so moc has a compilation unit to
// attach to) without turning the header into a heavy include.

namespace {

// Keeping a non-inline symbol here avoids "empty translation unit" warnings on
// some toolchains without inventing fake logic.
constexpr int kMediaItemTranslationUnitAnchor = 0;

} // namespace
