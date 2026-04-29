cmake_minimum_required(VERSION 3.16)
project(Zhazhashi_Survivor VERSION 1.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt6包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

set(SOURCES
    src/main.cpp
    src/gamewidget.cpp
    src/gameobject.cpp
    src/player.cpp
    src/enemy.cpp
    src/projectile.cpp
    src/weapon.cpp
    src/experiencegem.cpp
    src/spritemanager.cpp
)

set(HEADERS
    src/gamewidget.h
    src/gameobject.h
    src/player.h
    src/enemy.h
    src/projectile.h
    src/weapon.h
    src/experiencegem.h
    src/spritemanager.h
)

set(RESOURCES
    resources/resources.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    youxitubiao.rc






)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Windows下隐藏控制台窗口
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
