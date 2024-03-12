# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Set C++17 as the standard for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing with CTest
enable_testing()

project ("WovenSoftwarePlatformTest")

# Add source to this project's executable.
add_executable (SoftwarePlatformTest "src/buggy.cxx")
add_executable (UnitTests "tests/test.cpp")

# Add a run target to make running easier
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/SoftwarePlatformTest
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/SoftwarePlatformTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Add unit tests
add_test(NAME RunAllTests COMMAND UnitTests)