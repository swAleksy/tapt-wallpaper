cmake_minimum_required(VERSION 3.16)
project(tapt-wallpaper VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# Zauważ, że Core jest wymagane, a reszta idzie do głównej aplikacji
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Concurrent ShaderTools QmlIntegration DBus)
find_package(KF6 REQUIRED COMPONENTS Kirigami)

include(KDEInstallDirs6)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

# ==========================================
# 1. TARGET CORE (Zero zależności od GUI/QML)
# ==========================================
qt_add_library(taptwallpaper-core STATIC
    src/models/editstate.h
    src/models/queueitem.h
    src/models/playlistenums.h
    src/models/monitorplayliststate.h
    src/models/queuemodel.h  src/models/queuemodel.cpp

    src/services/playlistio.h src/services/playlistio.cpp
)

target_link_libraries(taptwallpaper-core PUBLIC
    Qt6::Core
    Qt6::QmlIntegration
)

target_include_directories(taptwallpaper-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

# ==========================================
# 2. TARGET GŁÓWNY (Aplikacja GUI)
# ==========================================
qt_add_executable(tapt-wallpaper
    src/main.cpp
)

qt_add_qml_module(tapt-wallpaper
    URI "org.kde.taptwallpaper"
    VERSION 1.0
    QML_FILES
      qml/Main.qml
      qml/DetailView.qml
      qml/MonitorPicker.qml
      qml/PlaylistItemRenderer.qml

      qml/common/OrderModeSelector.qml
      qml/common/PreviewImage.qml

      qml/gallery/Gallery.qml
      qml/gallery/GalleryGrid.qml

      qml/timeline/DayOfWeekTrack.qml
      qml/timeline/ScheduleTrack.qml
      qml/timeline/SimpleQueueList.qml
      qml/timeline/TimelinePanel.qml

    SOURCES
        src/models/imageItem.h
        src/models/imagesmodel.h            src/models/imagesmodel.cpp
        src/models/lutfilterslistmodel.h    src/models/lutfilterslistmodel.cpp

        src/services/galleryservice.h       src/services/galleryservice.cpp
        src/services/asyncimageprovider.h
        src/services/lutimageprovider.h
        src/services/lutservice.h           src/services/lutservice.cpp

        src/viewmodels/galleryviewmodel.h   src/viewmodels/galleryviewmodel.cpp
        src/viewmodels/detailviewmodel.h    src/viewmodels/detailviewmodel.cpp
        src/viewmodels/timelineviewmodel.h  src/viewmodels/timelineviewmodel.cpp
)

qt_add_shaders(tapt-wallpaper "shaders"
    FILES shaders/lut_filters.frag
)

file(GLOB LUT_FILES CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    luts/*
)

qt_add_resources(tapt-wallpaper "lut_resources"
    PREFIX "/"
    FILES ${LUT_FILES}
)

target_include_directories(tapt-wallpaper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodels
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
)

target_link_libraries(tapt-wallpaper PRIVATE
    taptwallpaper-core   # <--- Linkowanie logiki współdzielonej!
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Concurrent
    KF6::Kirigami
)

qt_generate_foreign_qml_types(taptwallpaper-core tapt-wallpaper)

# 3. TARGET DAEMON
qt_add_executable(taptwallpaper-daemond
    daemon/main.cpp
    daemon/wallpaperdaemon.h    daemon/wallpaperdaemon.cpp
    # daemon/wallpaperapplier.h   daemon/wallpaperapplier.cpp
)

target_include_directories(taptwallpaper-daemond PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/daemon
)

target_link_libraries(taptwallpaper-daemond PRIVATE
    taptwallpaper-core
    Qt6::Core
    Qt6::Gui
    Qt6::DBus
)
