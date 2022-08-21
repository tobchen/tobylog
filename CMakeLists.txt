cmake_minimum_required(VERSION 3.10)

project("tobylog")

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)

add_library(tobylog
    src/label.c
    src/text.c
    src/tobylog.c
)
target_compile_options(tobylog PUBLIC -g -Wall -Wextra -ansi -pedantic)

add_executable(label
    examples/label.c
)
target_link_libraries(label PUBLIC tobylog ${NCURSES_LIBRARIES})
target_compile_options(label PUBLIC -g -Wall -Wextra -ansi -pedantic)
