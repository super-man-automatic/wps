cmake_minimum_required(VERSION 3.20)
project(SmartDoc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找所需包
find_package(Qt6 COMPONENTS Core Gui Widgets Network Concurrent REQUIRED)
find_package(GTest REQUIRED)

# 项目源文件
set(SOURCES
    src/main.cpp
    src/gui/mainwindow.cpp
    src/gui/editor_widget.cpp
    src/gui/ai_sidebar.cpp
    src/core/document.cpp
    src/core/parser.cpp
    src/ai/llm_client.cpp
    src/ai/prompt_manager.cpp
    src/ai/ai_writer.cpp
)

set(HEADERS
    src/gui/mainwindow.h
    src/gui/editor_widget.h
    src/gui/ai_sidebar.h
    src/core/document.h
    src/core/parser.h
    src/ai/llm_client.h
    src/ai/prompt_manager.h
    src/ai/ai_writer.h
)

# 创建可执行文件
add_executable(SmartDoc ${SOURCES} ${HEADERS})

# Ensure target architecture matches the generator/platform (prefer 64-bit)
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Building for 32-bit target; this project expects 64-bit. Consider using a 64-bit generator/toolset.")
endif()

# 链接库
target_link_libraries(SmartDoc
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

# 包含目录
target_include_directories(SmartDoc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
    # Ensure exception handling model is enabled for MSVC (/EHsc)
    # Force UTF-8 encoding for source files and execution character set
    target_compile_options(SmartDoc PRIVATE /EHsc /utf-8)
endif()

# 测试
enable_testing()
add_subdirectory(tests)

# 安装目标
install(TARGETS SmartDoc DESTINATION bin)
