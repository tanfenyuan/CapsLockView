cmake_minimum_required(VERSION 4.2.0)
project(caps_tip)

include(${CMAKE_CURRENT_SOURCE_DIR}/CONFIG/qt6-win-tfy.cmake)

add_executable(${PROJECT_NAME} WIN32)

target_sources(${PROJECT_NAME}
    PRIVATE
        "src/main.cpp"
        "src/RawKeyboardMonitor.h"
        "src/RawkeyboardMonitor.cpp"
        "src/OsdWidget.h"
        "src/OsdWidget.cpp"
        "src/resources.qrc"
)


target_link_libraries(${PROJECT_NAME} 
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::SvgWidgets
        user32
)
