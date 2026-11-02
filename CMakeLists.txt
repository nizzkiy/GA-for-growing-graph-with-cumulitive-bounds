cmake_minimum_required(VERSION 3.15)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 1. Поиск OpenMP
find_package(OpenMP REQUIRED)

# 2. Поиск MPI (опционально)
option(USE_MPI "Enable MPI support" OFF)
if(USE_MPI)
    find_package(MPI REQUIRED)
    add_definitions(-DUSE_MPI)
endif()

# Форсируем сборку mimalloc как статической библиотеки
set(MI_BUILD_SHARED OFF CACHE INTERNAL "")
set(MI_BUILD_STATIC ON CACHE INTERNAL "")
set(MI_OVERRIDE OFF CACHE INTERNAL "") # Отключаем глобальный перехват, так как вы используете mimalloc-override.h вручную

# 3. Подключение mimalloc (сохраняет скорость выделения памяти)
include(FetchContent)
FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG        v2.1.2 # или другая стабильная версия
)
FetchContent_MakeAvailable(mimalloc)

# ========================================================
# ЦЕЛЬ 1: Сборка основного исполняемого файла (main.exe)
# ========================================================
add_executable(MyProject
    main.cpp
    Graph.cpp
    GraphUtil.cpp
)

target_link_libraries(MyProject PUBLIC 
    OpenMP::OpenMP_CXX 
    mimalloc-static
)

# Указываем компилятору, где искать заголовочные файлы mimalloc
target_include_directories(MyProject PUBLIC 
    ${mimalloc_SOURCE_DIR}/include
)

if(USE_MPI)
    target_link_libraries(MyProject PUBLIC MPI::MPI_CXX)
endif()

# ========================================================
# ЦЕЛЬ 2: Сборка DLL-библиотеки для вызова из Python
# ========================================================
add_library(network_api SHARED
    python_api.cpp     # Тот самый файл-мост с extern "C"
    Graph.cpp
    GraphUtil.cpp
)

target_link_libraries(network_api PUBLIC 
    OpenMP::OpenMP_CXX 
    mimalloc-static
)

# Указываем компилятору, где искать заголовочные файлы mimalloc
target_include_directories(network_api PUBLIC 
    ${mimalloc_SOURCE_DIR}/include
)

if(USE_MPI)
    target_link_libraries(network_api PUBLIC MPI::MPI_CXX)
endif()

# ========================================================
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyProject)