cmake_minimum_required(VERSION 3.10)

project("tobylog")

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

add_library(tobylog
    src/label.c
    src/text.c
    src/tobylog.c
)
target_include_directories(tobylog PUBLIC ${GLIB2_INCLUDE_DIRS})
target_compile_options(tobylog PUBLIC -g -Wall -Wextra -pedantic)

add_executable(label
    examples/label.c
)
target_include_directories(label PUBLIC ${GLIB2_INCLUDE_DIRS})
target_link_libraries(label PUBLIC tobylog ${GLIB2_LIBRARIES} ${NCURSES_LIBRARIES})
target_compile_options(label PUBLIC -g -Wall -Wextra -pedantic)

add_executable(text
    examples/text.c
)
target_include_directories(text PUBLIC ${GLIB2_INCLUDE_DIRS})
target_link_libraries(text PUBLIC tobylog ${GLIB2_LIBRARIES} ${NCURSES_LIBRARIES})
target_compile_options(text PUBLIC -g -Wall -Wextra -pedantic)
