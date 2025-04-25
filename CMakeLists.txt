cmake_minimum_required(VERSION 3.16)

project(MultiMediaDemo
    VERSION 0.1
    LANGUAGES CXX
    DESCRIPTION "A multimedia player application"
)

# 基础配置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX "d")  # Debug版本添加后缀

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

# TagLib 配置
add_definitions(-DTAGLIB_STATIC)
set(TAGLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/taglib/include")
set(TAGLIB_LIBRARY "${CMAKE_SOURCE_DIR}/3rdparty/taglib/lib/tag.lib")

# 查找Qt组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

# 源文件定义
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# 创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MultiMediaDemo
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(MultiMediaDemo SHARED ${PROJECT_SOURCES})
    else()
        add_executable(MultiMediaDemo ${PROJECT_SOURCES})
    endif()
endif()

# 链接库
target_link_libraries(MultiMediaDemo PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${TAGLIB_LIBRARY}
)

# 包含目录
target_include_directories(MultiMediaDemo PRIVATE
    ${TAGLIB_INCLUDE_DIR}
)

# 平台特定配置
if(APPLE)
    # macOS/iOS配置
    set_target_properties(MultiMediaDemo PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.MultiMediaDemo"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
elseif(WIN32)
    # Windows配置
    set_target_properties(MultiMediaDemo PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 安装配置
include(GNUInstallDirs)
install(TARGETS MultiMediaDemo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt6特定配置
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MultiMediaDemo)
endif()

# 生成编译命令数据库（可选，用于IDE支持）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 强制 Release 模式禁用优化并生成调试信息
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE
        "/Od /Zi /DEBUG /GL- /MD /DNDEBUG"
    )
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /OPT:REF /OPT:ICF")
endif()
