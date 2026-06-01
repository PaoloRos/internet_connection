cmake_minimum_required(VERSION 3.16)
project(internet_connection VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/usr/local" CACHE PATH "Install path prefix" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Network)

add_executable(internet_connection
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/ConnectivityChecker.h
    src/ConnectivityChecker.cpp
)

target_link_libraries(internet_connection PRIVATE
    Qt6::Widgets
    Qt6::Network
)

install(TARGETS internet_connection
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
