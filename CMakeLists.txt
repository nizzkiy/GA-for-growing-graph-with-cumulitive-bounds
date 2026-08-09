cmake_minimum_required(VERSION 3.15)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Автоматическая загрузка и подключение mimalloc ---
include(FetchContent)

FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG        v2.1.2 # Фиксированная стабильная версия
)

# Отключаем ненужные опции сборки mimalloc, чтобы ускорить компиляцию
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MI_BUILD_OBJECTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(mimalloc)

# --- Поиск внешних зависимостей ---
find_package(OpenMP REQUIRED)

option(USE_MPI "Enable MPI support" OFF)
if(USE_MPI)
    find_package(MPI REQUIRED)
    add_definitions(-DUSE_MPI)
endif()

# --- Исполняемый файл ---
add_executable(MyProject
    main.cpp
    Graph.cpp
    GraphUtil.cpp
)

# --- Линковка библиотек ---
target_link_libraries(MyProject 
    PUBLIC 
        OpenMP::OpenMP_CXX
        mimalloc-static
)

if(USE_MPI)
    target_link_libraries(MyProject PUBLIC MPI::MPI_CXX)
endif()

# Подключаем заголовки mimalloc к проекту
target_include_directories(MyProject PRIVATE ${mimalloc_SOURCE_DIR}/include)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyProject)