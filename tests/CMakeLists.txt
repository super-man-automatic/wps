cmake_minimum_required(VERSION 3.20)

# 单元测试
add_executable(SmartDocTests
    core_tests/test_document.cpp
    ai_tests/test_llm_client.cpp
)

target_link_libraries(SmartDocTests
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Network
)

target_include_directories(SmartDocTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

add_test(NAME SmartDocTests COMMAND SmartDocTests)
