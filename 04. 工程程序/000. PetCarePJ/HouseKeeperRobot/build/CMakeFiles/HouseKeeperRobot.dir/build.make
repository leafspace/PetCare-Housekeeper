# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build"

# Include any dependencies generated for this target.
include CMakeFiles/HouseKeeperRobot.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/HouseKeeperRobot.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/HouseKeeperRobot.dir/flags.make

CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o: CMakeFiles/HouseKeeperRobot.dir/flags.make
CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o: ../Drive/DriveCamera/DriveCamera.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o -c "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera/DriveCamera.cpp"

CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera/DriveCamera.cpp" > CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.i

CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera/DriveCamera.cpp" -o CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.s

CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.requires:

.PHONY : CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.requires

CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.provides: CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.requires
	$(MAKE) -f CMakeFiles/HouseKeeperRobot.dir/build.make CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.provides.build
.PHONY : CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.provides

CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.provides.build: CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o


CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o: CMakeFiles/HouseKeeperRobot.dir/flags.make
CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o: ../SourceMain.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o -c "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/SourceMain.cpp"

CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/SourceMain.cpp" > CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.i

CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/SourceMain.cpp" -o CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.s

CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.requires:

.PHONY : CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.requires

CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.provides: CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.requires
	$(MAKE) -f CMakeFiles/HouseKeeperRobot.dir/build.make CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.provides.build
.PHONY : CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.provides

CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.provides.build: CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o


# Object files for target HouseKeeperRobot
HouseKeeperRobot_OBJECTS = \
"CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o" \
"CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o"

# External object files for target HouseKeeperRobot
HouseKeeperRobot_EXTERNAL_OBJECTS =

HouseKeeperRobot: CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o
HouseKeeperRobot: CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o
HouseKeeperRobot: CMakeFiles/HouseKeeperRobot.dir/build.make
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_videostab.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_ts.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_superres.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_stitching.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_ocl.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_gpu.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_contrib.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_photo.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_legacy.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_video.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_objdetect.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_ml.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_calib3d.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_features2d.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_highgui.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_imgproc.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_flann.so.2.4.9
HouseKeeperRobot: /usr/lib/arm-linux-gnueabihf/libopencv_core.so.2.4.9
HouseKeeperRobot: CMakeFiles/HouseKeeperRobot.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable HouseKeeperRobot"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/HouseKeeperRobot.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/HouseKeeperRobot.dir/build: HouseKeeperRobot

.PHONY : CMakeFiles/HouseKeeperRobot.dir/build

CMakeFiles/HouseKeeperRobot.dir/requires: CMakeFiles/HouseKeeperRobot.dir/Drive/DriveCamera/DriveCamera.cpp.o.requires
CMakeFiles/HouseKeeperRobot.dir/requires: CMakeFiles/HouseKeeperRobot.dir/SourceMain.cpp.o.requires

.PHONY : CMakeFiles/HouseKeeperRobot.dir/requires

CMakeFiles/HouseKeeperRobot.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/HouseKeeperRobot.dir/cmake_clean.cmake
.PHONY : CMakeFiles/HouseKeeperRobot.dir/clean

CMakeFiles/HouseKeeperRobot.dir/depend:
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/CMakeFiles/HouseKeeperRobot.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/HouseKeeperRobot.dir/depend

