cmake_minimum_required(VERSION 3.21)

project(FenSuCloudAlbum
    VERSION 0.1.0
    DESCRIPTION "FenSu Cloud Album - WebDAV backed cloud photo/video album"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global C++ standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Qt discovery
# ---------------------------------------------------------------------------
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Network
    Sql
    Svg
)

qt_standard_project_setup(REQUIRES 6.5)

# ---------------------------------------------------------------------------
# Qt policies
# ---------------------------------------------------------------------------
# QTP0002 governs android-specific target properties: they may contain
# generator expressions but must evaluate to valid JSON. Without an explicit
# setting, Qt 6.7 prints a developer warning on every Android configure.
# qt_policy is only available after Qt has been found.
if(COMMAND qt_policy)
    qt_policy(SET QTP0002 NEW)
endif()

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
set(APP_SOURCES
    src/main.cpp

    # app layer
    src/app/AppController.cpp

    # core / database
    src/core/database/DatabaseManager.cpp
    src/core/database/DatabaseSchema.cpp

    # core / model  (value types + list models)
    src/core/model/MediaItem.cpp
    src/core/model/MediaModel.cpp
    src/core/model/AlbumModel.cpp
    src/core/model/TimelineModel.cpp
    src/core/model/MemoryModel.cpp

    # core / cache
    src/core/cache/CacheManager.cpp

    # core / network
    src/core/network/NetworkMonitor.cpp
)

set(PLATFORM_SOURCES
    src/platform/android/AndroidPlatform.cpp
)

set(APP_HEADERS
    src/app/AppController.h

    src/core/database/DatabaseManager.h
    src/core/database/DatabaseSchema.h

    src/core/model/MediaItem.h
    src/core/model/MediaModel.h
    src/core/model/AlbumItem.h
    src/core/model/AlbumModel.h
    src/core/model/TimelineModel.h
    src/core/model/MemoryModel.h

    src/core/cache/CacheManager.h

    src/core/network/NetworkMonitor.h

    src/platform/android/AndroidPlatform.h
)

qt_add_executable(FenSuCloudAlbum
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${PLATFORM_SOURCES}
)

# ---------------------------------------------------------------------------
# QML module
# ---------------------------------------------------------------------------
# The QML sources live under src/ui/qml/ in several sub directories:
# theme/, components/ and pages/.
#
# How the paths below end up in the resource system
# ------------------------------------------------
# qt_add_qml_module() maps every QML_FILES entry to
#     <RESOURCE_PREFIX>/<URI with dots as slashes>/<path as written>
# where the path is recorded relative to the directory CMakeLists.txt lives in
# (CMAKE_CURRENT_SOURCE_DIR). The files are therefore written here exactly as
# they appear in the repository and main.cpp loads exactly the same relative
# path:
#
#     QML_FILES "src/ui/qml/Main.qml"
#  -> qrc:/qt/qml/FenSuCloudAlbum/src/ui/qml/Main.qml
#
# RESOURCE_PREFIX is stated explicitly rather than relying on the default.
# Qt 6.5 introduced QTP0001, which already defaults it to /qt/qml, but Qt 6.5
# and later emit a configure warning when neither RESOURCE_PREFIX nor
# NO_RESOURCE_TARGET_PATH is given. Being explicit silences that and documents
# the intent.
#
# Note: there is no QT_QML_SOURCE_DIRECTORY argument. It does not exist in
# qt_add_qml_module() and passing it makes configure fail with
# "Unknown/unexpected arguments". The module root cannot be relocated; the
# directory layout of QML_FILES is what determines the resource paths.
#

# Theme.qml is declared `pragma Singleton`. Without this property the
# generated qmldir has no singleton entry and every `Theme.xxx` reference in
# QML fails to resolve at runtime.
set_source_files_properties(src/ui/qml/theme/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(FenSuCloudAlbum
    URI FenSuCloudAlbum
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        src/ui/qml/Main.qml
        src/ui/qml/theme/Theme.qml
        src/ui/qml/components/BottomNavBar.qml
        src/ui/qml/components/PhotoGrid.qml
        src/ui/qml/components/AlbumCard.qml
        src/ui/qml/components/SectionHeader.qml
        src/ui/qml/pages/AlbumsPage.qml
        src/ui/qml/pages/AllPhotosPage.qml
        src/ui/qml/pages/MemoriesPage.qml
)

# ---------------------------------------------------------------------------
# Linking
# ---------------------------------------------------------------------------
target_include_directories(FenSuCloudAlbum PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(FenSuCloudAlbum PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
    Qt6::Svg
)

# ---------------------------------------------------------------------------
# Platform definitions
# ---------------------------------------------------------------------------
if(ANDROID)
    set_target_properties(FenSuCloudAlbum PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
    target_compile_definitions(FenSuCloudAlbum PRIVATE FSCA_PLATFORM_ANDROID)
elseif(WIN32)
    target_compile_definitions(FenSuCloudAlbum PRIVATE FSCA_PLATFORM_WINDOWS)
else()
    target_compile_definitions(FenSuCloudAlbum PRIVATE FSCA_PLATFORM_LINUX)
endif()

install(TARGETS FenSuCloudAlbum
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)