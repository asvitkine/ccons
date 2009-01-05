cmake_minimum_required(VERSION 2.6)

Project(ccons)

add_executable(ccons ccons.cpp)

set(LIBS ${LIBS} /usr/local/lib/libctemplate.a)
set(LIBS ${LIBS} /usr/local/lib/libedit.a -lcurses)
target_link_libraries(ccons ${LIBS})
