# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\artio\projectsvscode\mp_os

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\artio\projectsvscode\mp_os\build

# Include any dependencies generated for this target.
include allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/compiler_depend.make

# Include the progress variables for this target.
include allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/progress.make

# Include the compile flags for this target's objects.
include allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/flags.make

allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/flags.make
allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/includes_CXX.rsp
allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj: C:/Users/artio/projectsvscode/mp_os/allocator/allocator_red_black_tree/tests/allocator_red_black_tree_tests.cpp
allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\artio\projectsvscode\mp_os\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj"
	cd /d C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests && C:\msys64\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj -MF CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\allocator_red_black_tree_tests.cpp.obj.d -o CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\allocator_red_black_tree_tests.cpp.obj -c C:\Users\artio\projectsvscode\mp_os\allocator\allocator_red_black_tree\tests\allocator_red_black_tree_tests.cpp

allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.i"
	cd /d C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests && C:\msys64\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\artio\projectsvscode\mp_os\allocator\allocator_red_black_tree\tests\allocator_red_black_tree_tests.cpp > CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\allocator_red_black_tree_tests.cpp.i

allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.s"
	cd /d C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests && C:\msys64\ucrt64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\artio\projectsvscode\mp_os\allocator\allocator_red_black_tree\tests\allocator_red_black_tree_tests.cpp -o CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\allocator_red_black_tree_tests.cpp.s

# Object files for target mp_os_allctr_allctr_rb_tr_tests
mp_os_allctr_allctr_rb_tr_tests_OBJECTS = \
"CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj"

# External object files for target mp_os_allctr_allctr_rb_tr_tests
mp_os_allctr_allctr_rb_tr_tests_EXTERNAL_OBJECTS =

allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/allocator_red_black_tree_tests.cpp.obj
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/build.make
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: lib/libgtest_main.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: common/libmp_os_cmmn.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: logger/client_logger/libmp_os_lggr_clnt_lggr.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator/libmp_os_allctr_allctr.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator_red_black_tree/libmp_os_allctr_allctr_rb_tr.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: lib/libgtest.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: common/libmp_os_cmmn.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator/libmp_os_allctr_allctr.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: logger/logger/libmp_os_lggr_lggr.a
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/linkLibs.rsp
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/objects1.rsp
allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe: allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\artio\projectsvscode\mp_os\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable mp_os_allctr_allctr_rb_tr_tests.exe"
	cd /d C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/build: allocator/allocator_red_black_tree/tests/mp_os_allctr_allctr_rb_tr_tests.exe
.PHONY : allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/build

allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/clean:
	cd /d C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests && $(CMAKE_COMMAND) -P CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\cmake_clean.cmake
.PHONY : allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/clean

allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\artio\projectsvscode\mp_os C:\Users\artio\projectsvscode\mp_os\allocator\allocator_red_black_tree\tests C:\Users\artio\projectsvscode\mp_os\build C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests C:\Users\artio\projectsvscode\mp_os\build\allocator\allocator_red_black_tree\tests\CMakeFiles\mp_os_allctr_allctr_rb_tr_tests.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : allocator/allocator_red_black_tree/tests/CMakeFiles/mp_os_allctr_allctr_rb_tr_tests.dir/depend

