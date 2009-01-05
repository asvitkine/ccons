cmake_minimum_required(VERSION 2.6)

Project(ccons)

add_executable(ccons ccons.cpp)

target_link_libraries(ccons /usr/local/lib/libctemplate.a)
