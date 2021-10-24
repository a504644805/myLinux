cmake_minimum_required(VERSION 3.20)
project(student_management_sys)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-fpermissive")

add_executable(student_management_sys main.cpp list.h student.h student.cpp)