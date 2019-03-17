cmake_minimum_required(VERSION 3.5)
PROJECT(HouseKeeperRobot)
find_package(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(
include
)

aux_source_directory(. SRC_LIST)
add_library(demo ${SRC_LIST})

ADD_EXECUTABLE(${PROJECT_NAME} SourceMain.cpp)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenCV_LIBS})
