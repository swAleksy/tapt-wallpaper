cmake_minimum_required(VERSION 3.16)
project(tapt-wallpaper VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # dla clangd/VSCode

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(QT_MAJOR_VERSION 6)
set(ECM_QMAKE_EXECUTABLE /usr/bin/qmake6)

include(KDEInstallDirs6)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets)
find_package(KF6 REQUIRED COMPONENTS Kirigami)

qt_add_executable(tapt-wallpaper
    src/main.cpp
)

qt_add_qml_module(tapt-wallpaper
    URI "org.kde.taptwallpaper"
    VERSION 1.0
    QML_FILES qml/Main.qml
    QML_FILES qml/Gallery.qml
    QML_FILES qml/GalleryGrid.qml
    QML_FILES qml/GalleryFooter.qml
    SOURCES 
        src/models/imageItem.h
        src/services/galleryservice.h src/services/galleryservice.cpp
        src/models/imagemodel.h       src/models/imagemodel.cpp
        src/controllers/gallerycontroller.h src/controllers/gallerycontroller.cpp 
)

target_include_directories(tapt-wallpaper PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
)

target_link_libraries(tapt-wallpaper PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets 
    KF6::Kirigami
)