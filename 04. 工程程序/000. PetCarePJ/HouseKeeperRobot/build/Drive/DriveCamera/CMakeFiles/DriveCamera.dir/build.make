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
include Drive/DriveCamera/CMakeFiles/DriveCamera.dir/depend.make

# Include the progress variables for this target.
include Drive/DriveCamera/CMakeFiles/DriveCamera.dir/progress.make

# Include the compile flags for this target's objects.
include Drive/DriveCamera/CMakeFiles/DriveCamera.dir/flags.make

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/flags.make
Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o: ../Drive/DriveCamera/DriveCamera.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o"
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o -c "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera/DriveCamera.cpp"

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DriveCamera.dir/DriveCamera.cpp.i"
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera/DriveCamera.cpp" > CMakeFiles/DriveCamera.dir/DriveCamera.cpp.i

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DriveCamera.dir/DriveCamera.cpp.s"
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera/DriveCamera.cpp" -o CMakeFiles/DriveCamera.dir/DriveCamera.cpp.s

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.requires:

.PHONY : Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.requires

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.provides: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.requires
	$(MAKE) -f Drive/DriveCamera/CMakeFiles/DriveCamera.dir/build.make Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.provides.build
.PHONY : Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.provides

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.provides.build: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o


# Object files for target DriveCamera
DriveCamera_OBJECTS = \
"CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o"

# External object files for target DriveCamera
DriveCamera_EXTERNAL_OBJECTS =

Drive/DriveCamera/libDriveCamera.a: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o
Drive/DriveCamera/libDriveCamera.a: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/build.make
Drive/DriveCamera/libDriveCamera.a: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libDriveCamera.a"
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" && $(CMAKE_COMMAND) -P CMakeFiles/DriveCamera.dir/cmake_clean_target.cmake
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DriveCamera.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Drive/DriveCamera/CMakeFiles/DriveCamera.dir/build: Drive/DriveCamera/libDriveCamera.a

.PHONY : Drive/DriveCamera/CMakeFiles/DriveCamera.dir/build

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/requires: Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DriveCamera.cpp.o.requires

.PHONY : Drive/DriveCamera/CMakeFiles/DriveCamera.dir/requires

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/clean:
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" && $(CMAKE_COMMAND) -P CMakeFiles/DriveCamera.dir/cmake_clean.cmake
.PHONY : Drive/DriveCamera/CMakeFiles/DriveCamera.dir/clean

Drive/DriveCamera/CMakeFiles/DriveCamera.dir/depend:
	cd "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/Drive/DriveCamera" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera" "/home/leafspace/PetCare-Housekeeper/04. 工程程序/000. PetCarePJ/HouseKeeperRobot/build/Drive/DriveCamera/CMakeFiles/DriveCamera.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : Drive/DriveCamera/CMakeFiles/DriveCamera.dir/depend

