message(STATUS  "USING QT WIN")


set(CMAKE_PREFIX_PATH ${QT_ROOT_PATH})


set(CMAKE_AUTOGEN_BUILD_DIR "${CMAKE_BINARY_DIR}/autogen")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Svg
    SvgWidgets
)

# TARGET_LINK_LIBRARIES( PRIVATE Qt::Core, Qt6::Widgets user32)
