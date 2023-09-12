cmake_minimum_required(VERSION 3.25)
project(PainlessMeshLib)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost 1.42.0 REQUIRED COMPONENTS filesystem system thread regex program_options date_time)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF (Boost_FOUND)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_library(PainlessMeshLib STATIC src/fake_serial.cpp libs/painlessMesh/src/scheduler.cpp src/painless_mesh.cpp src/ota.cpp)

TARGET_LINK_LIBRARIES(PainlessMeshLib ${Boost_LIBRARIES})

target_include_directories(PainlessMeshLib PUBLIC src/
        libs/TaskScheduler/src/
        libs/ArduinoJson/
        libs/painlessMesh/src/
        libs/uuid/include/)

