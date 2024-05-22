cmake_minimum_required(VERSION 3.16)
project(CamCapture LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia MultimediaWidgets Widgets)

qt_add_executable(CamCapture
    MANUAL_FINALIZATION
    cameracapturemain.cpp cameracapturemain.h cameracapturemain.ui

    main.cpp
    settings.cpp settings.h settings.ui

    .gitignore
    about.h about.cpp about.ui
    camera.qrc
    cameracapture.h cameracapture.cpp
    help.html
)

set_target_properties(CamCapture PROPERTIES
    WIN32_EXECUTABLE TRUE
)

set(CamCapture_resource_files
    "images/shutter.svg"
    "images/stop.svg"
    "images/record.svg"
    "images/pause.svg"
    "images/mic-on.svg"
    "images/mic-off.svg"
    "images/info.svg"
    "images/logo.svg"
    "images/grid33.svg"
    "images/square.svg"
    "images/crosshair.svg"
    "images/goldenratio.svg"
    "help.html"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/help.html
  ${CMAKE_BINARY_DIR}/help.html COPYONLY)

qt_add_resources(CamCapture "CamCapture"
    PREFIX
        "/"
    FILES
        ${CamCapture_resource_files}
)

target_link_libraries(CamCapture PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::MultimediaWidgets
    Qt::Widgets
)

qt_finalize_executable(CamCapture)
