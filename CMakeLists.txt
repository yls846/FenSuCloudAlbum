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
# The QML sources live under src/ui/qml/ in several sub directories
# (theme/, components/, pages/). Two properties make the generated qmldir and
# the resource layout match what main.cpp loads:
#
#   QT_QML_SOURCE_DIRECTORY  tells Qt which directory is the module root, so
#                            the files are registered as  Main.qml,
#                            components/PhotoGrid.qml, ... rather than as
#                            src/ui/qml/components/PhotoGrid.qml.
#
#   RESOURCE_PREFIX          keeps the well known  /qt/qml  prefix, so the
#                            module ends up at
#                            qrc:/qt/qml/FenSuCloudAlbum/  which is exactly
#                            the URL main.cpp requests for Main.qml.
#
# Without QT_QML_SOURCE_DIRECTORY, Qt 6.5+ cannot deduce the module root from
# multi directory QML_FILES lists and configuration fails.
#
set(FSCA_QML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/qml")

# Theme.qml is declared `pragma Singleton`. Without this property the
# generated qmldir has no singleton entry and every `Theme.xxx` reference in
# QML fails to resolve at runtime.
set_source_files_properties("${FSCA_QML_DIR}/theme/Theme.qml"
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(FenSuCloudAlbum
    URI FenSuCloudAlbum
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QT_QML_SOURCE_DIRECTORY "${FSCA_QML_DIR}"
    QML_FILES
        "${FSCA_QML_DIR}/Main.qml"
        "${FSCA_QML_DIR}/theme/Theme.qml"
        "${FSCA_QML_DIR}/components/BottomNavBar.qml"
        "${FSCA_QML_DIR}/components/PhotoGrid.qml"
        "${FSCA_QML_DIR}/components/AlbumCard.qml"
        "${FSCA_QML_DIR}/components/SectionHeader.qml"
        "${FSCA_QML_DIR}/pages/AlbumsPage.qml"
        "${FSCA_QML_DIR}/pages/AllPhotosPage.qml"
        "${FSCA_QML_DIR}/pages/MemoriesPage.qml"
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