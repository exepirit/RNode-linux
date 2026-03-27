cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS_DEBUG "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O3")

project(rnode)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIOd REQUIRED libgpiod)

find_library(LIBCYAML_LIBRARY NAMES cyaml)
find_library(LIBYAML_LIBRARY NAMES yaml)

include_directories(${LIBGPIOd_INCLUDE_DIRS})
include_directories(/usr/include)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${LIBCYAML_LIBRARY}
    ${LIBYAML_LIBRARY}
    ${LIBGPIOd_LIBRARIES}
    m)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/sx126x)

add_subdirectory(src)
add_subdirectory(sx126x)

install(FILES rnode.yaml DESTINATION etc)
install(TARGETS ${PROJECT_NAME} DESTINATION sbin)
