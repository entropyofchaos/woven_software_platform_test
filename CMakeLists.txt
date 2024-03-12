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
add_executable (UnitTestsTSan "tests/test.cpp")
add_executable (UnitTestsASan "tests/test.cpp")


# Set -Werror to catch "argument unused during compilation" warnings
include(CheckCXXCompilerFlag)
set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=thread") # Also needs to be a link flag for test to pass
check_cxx_compiler_flag("-fsanitize=thread" COMPILER_SUPPORTS_TSAN)
set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=address") # Also needs to be a link flag for test to pass
check_cxx_compiler_flag("-fsanitize=address" COMPILER_SUPPORTS_ASAN)
unset(CMAKE_REQUIRED_FLAGS)

# Check if the compiler supports thread sanitizer and if it does, 
# add it to the respective unit test executables
if(COMPILER_SUPPORTS_TSAN)
    target_compile_options(UnitTestsTSan PRIVATE -fsanitize=thread -fPIE -pie)
    target_link_options(UnitTestsTSan PRIVATE -fsanitize=thread)
else()
    message(STATUS "Thread Sanitizer is not supported by the compiler.")
endif()

# Check if the compiler supports taddress sanitizer and if it does, 
# add it to the respective unit test executables
if(COMPILER_SUPPORTS_ASAN)
    target_compile_options(UnitTestsASan PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(UnitTestsASan PRIVATE -fsanitize=address)
else()
    message(STATUS "Address Sanitizer is not supported by the compiler.")
endif()

# Add a run target to make running easier
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/SoftwarePlatformTest
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/SoftwarePlatformTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Add unit tests
add_test(NAME RunAllTests COMMAND UnitTests)
add_test(NAME RunAllTestsTSan COMMAND UnitTestsTSan)
add_test(NAME RunAllTestsASan COMMAND UnitTestsASan)