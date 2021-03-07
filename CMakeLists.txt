cmake_minimum_required(VERSION 3.5)

project (painlessMeshBoost)

# Find python and Boost - both are required dependencies
#find_package(PythonLibs 2.7 REQUIRED)
#find_package (Python3 COMPONENTS Interpreter Development)
find_package (Python3 COMPONENTS Development)

find_package(Boost COMPONENTS python REQUIRED)
#find_package(Python COMPONENTS Interpreter Development)
# Without this, any build libraries automatically have names "lib{x}.so"
set(CMAKE_SHARED_MODULE_PREFIX "")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

#FIND_PACKAGE(Boost 1.42.0 REQUIRED COMPONENTS filesystem system thread regex program_options python)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system thread regex program_options python)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)
message("Python_FOUND:${Python_FOUND}")
message("Python_VERSION:${Python_VERSION}")
message("Python_Development_FOUND:${Python_Development_FOUND}")
message("Python_LIBRARIES:${Python_LIBRARIES}")
message("Python_include_dirs:${PYTHON_INCLUDE_DIRS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Add a shared module - modules are intended to be imported at runtime.
# - This is where you add the source files
add_library(pmbPython_ext MODULE src/pmbPython.cpp src/fake_serial.cpp libs/painlessMesh/src/painlessMeshConnection.cpp libs/painlessMesh/src/scheduler.cpp)

target_include_directories(pmbPython_ext PUBLIC src/ libs/TaskScheduler/src/ libs/ArduinoJson/ libs/painlessMesh/src/ libs/uuid/include/ ${PYTHON_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES(pmbPython_ext ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})


ADD_EXECUTABLE(painlessMeshBoost src/main.cpp src/fake_serial.cpp libs/painlessMesh/src/painlessMeshConnection.cpp libs/painlessMesh/src/scheduler.cpp)
target_include_directories(painlessMeshBoost PUBLIC src/ libs/TaskScheduler/src/ libs/ArduinoJson/ libs/painlessMesh/src/ libs/uuid/include/)
TARGET_LINK_LIBRARIES(painlessMeshBoost ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})

install(TARGETS pmbPython_ext DESTINATION bin)
