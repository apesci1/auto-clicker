cmake_minimum_required(VERSION 3.5)

project(auto-clicker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Qt libraries
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(qhotkey REQUIRED)

# Set the project sources
set(PROJECT_SOURCES
    main.cpp
    autoclicker.cpp
    autoclicker.h
    autoclicker.ui
)

# Create the executable
qt_add_executable(auto-clicker
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# Link the Qt libraries
target_link_libraries(auto-clicker PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    qhotkey
)

set_target_properties(auto-clicker PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS auto-clicker
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(auto-clicker)

# Print message to confirm configuration
message(STATUS "Configuration complete, ready to build.")
