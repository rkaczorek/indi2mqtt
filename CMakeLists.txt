cmake_minimum_required(VERSION 2.8.12)
PROJECT(indi2mqtt CXX C)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

include(GNUInstallDirs)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

set(BIN_INSTALL_DIR "/usr/bin")
set(CONFIG_INSTALL_DIR "/etc")

find_package(INDI REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Mosquitto REQUIRED)
find_package(Threads REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})
include(CMakeCommon)

add_executable(indi2mqtt indi2mqtt.cpp)

target_link_libraries(indi2mqtt indiclient ${INDI_LIBRARIES} ${ZLIB_LIBRARY} ${MOSQUITTO_LIBRARY})

IF (UNIX AND NOT APPLE)
    target_link_libraries(indi2mqtt -lpthread)
ENDIF ()

install(TARGETS indi2mqtt RUNTIME DESTINATION bin )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/indi2mqtt.service DESTINATION ${CONFIG_INSTALL_DIR}/systemd/system)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/indi2mqtt.conf DESTINATION ${CONFIG_INSTALL_DIR})
