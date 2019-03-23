cmake_minimum_required(VERSION 2.6)
project(HouseKeeperRobot)
find_package(OpenCV REQUIRED)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} Drive/DriveCamera/DriveCamera.cpp SourceMain.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)