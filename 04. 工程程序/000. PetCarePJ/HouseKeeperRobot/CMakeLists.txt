cmake_minimum_required(VERSION 2.6)
project(HouseKeeperRobot)
find_package(OpenCV REQUIRED)

aux_source_directory(. SRC_LIST)
add_subdirectory(Drive/DriveCamera)
add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} DriveCamera)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
