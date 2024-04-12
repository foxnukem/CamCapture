cmake_minimum_required(VERSION 3.16)
project(CamCapture LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia MultimediaWidgets Widgets)

qt_add_executable(CamCapture
    MANUAL_FINALIZATION
    camera.cpp camera.h camera.ui

    main.cpp
    settings.cpp settings.h settings.ui
    metadatadialog.cpp metadatadialog.h
    .gitignore
)

set_target_properties(CamCapture PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(CamCapture PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::MultimediaWidgets
    Qt::Widgets
)

# Resources:
set(CamCapture_resource_files
    "images/shutter.svg"
)

qt_add_resources(CamCapture "CamCapture"
    PREFIX
        "/"
    FILES
        ${CamCapture_resource_files}
)

qt_finalize_executable(CamCapture)
