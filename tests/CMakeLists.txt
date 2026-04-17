cmake_minimum_required(VERSION 3.20)

# 单元测试
cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# 测试需要链接的源文件（排除 main.cpp 和 GUI 相关文件）
set(TEST_SOURCES
    ../src/core/document.cpp
    ../src/ai/llm_client.cpp
    ../src/ai/prompt_manager.cpp
    ../src/ai/ai_writer.cpp
    core_tests/test_document.cpp
    ai_tests/test_llm_client.cpp
)

add_executable(SmartDocTests ${TEST_SOURCES})

target_link_libraries(SmartDocTests
    gtest_main
    Qt6::Core
    Qt6::Network
)

if(MSVC)
    target_compile_options(SmartDocTests PRIVATE /EHsc /utf-8)
endif()

target_include_directories(SmartDocTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

add_test(NAME SmartDocTests COMMAND SmartDocTests)
