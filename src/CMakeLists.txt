cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE $ENV{HOME}/arm-cross-comp-env/rpi-zero-w-toolchain.cmake)
project(SETR_TP5)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SOURCES utils.c emulateurClavier.c tamponCirculaire.c main.c)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=armv6 -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -Ofast -funroll-loops -funsafe-math-optimizations -floop-block -flto")

SET(GCC_WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wshadow")
add_definitions("${GCC_WARNING_FLAGS}")

add_executable(emulateurClavier ${SOURCES})
target_link_libraries(emulateurClavier PRIVATE Threads::Threads m)
