project(crrc)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()
if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

enable_testing()

find_package(Qt5 COMPONENTS Core Network Sql REQUIRED)
find_package(CutelystQt5 REQUIRED)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable C++14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CutelystQt5_INCLUDE_DIR}
)

file(GLOB_RECURSE TEMPLATES_SRC root/*)

add_subdirectory(src)

find_package(Qt5Test REQUIRED)
add_subdirectory(tests/unit)
add_test(crrcTest crrcTest)
