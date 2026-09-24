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
)

# Platform specific sources.
# AndroidPlatform is always compiled: the .cpp is internally guarded with
# Q_OS_ANDROID so the same API exists on desktop, which lets main.cpp and the
# application layer call it unconditionally.
set(PLATFORM_SOURCES
    src/platform/android/AndroidPlatform.cpp
)

list(APPEND APP_HEADERS
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
# Theme.qml is declared `pragma Singleton`. Without marking it here, the
# generated qmldir never contains the singleton registration and every
# `Theme.xxx` reference in QML fails to resolve at runtime.
set_source_files_properties(src/ui/qml/theme/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(FenSuCloudAlbum
    URI FenSuCloudAlbum
    VERSION 1.0
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
    RESOURCES
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
