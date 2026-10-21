cmake_minimum_required(VERSION 4.2.3)
project(lab2 LANGUAGES C)
set(CMAKE_C_STANDARD 11)

add_executable(parent parent.c)
add_executable(child child.c)

# cd src/
# cmake -S . -B build
# cmake --build build
