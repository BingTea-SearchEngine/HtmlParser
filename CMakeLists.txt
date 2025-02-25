cmake_minimum_required(VERSION 3.31)

set(THIS HtmlParser)

project(${THIS} CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")

add_library(${THIS} STATIC ${LIB_DIR}/Parser.cpp ${LIB_DIR}/HtmlTags.cpp)
target_include_directories(${THIS} PUBLIC ${LIB_DIR})

add_executable(parserMain src/main.cpp)
target_link_libraries(parserMain PUBLIC ${THIS})

add_executable(ParserTests tests/tests.cpp)
target_link_libraries(ParserTests PRIVATE ${THIS} GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ParserTests)

