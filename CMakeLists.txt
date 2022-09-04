cmake_minimum_required(VERSION 3.10)

project("tobylog")

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)
pkg_check_modules(APR REQUIRED apr-1)

add_library(tobylog
    src/label.c
    src/string.c
    src/text.c
    src/tobylog.c
    src/utf8.c
)
target_include_directories(tobylog PUBLIC ${APR_INCLUDE_DIRS})
target_compile_options(tobylog PUBLIC -g -Wall -Wextra -pedantic)

add_executable(label
    examples/label.c
)
target_include_directories(label PUBLIC ${APR_INCLUDE_DIRS})
target_link_libraries(label PUBLIC tobylog ${APR_LIBRARIES} ${NCURSES_LIBRARIES})
target_compile_options(label PUBLIC -g -Wall -Wextra -pedantic)

add_executable(text
    examples/text.c
)
target_include_directories(text PUBLIC ${APR_INCLUDE_DIRS})
target_link_libraries(text PUBLIC tobylog ${APR_LIBRARIES} ${NCURSES_LIBRARIES})
target_compile_options(text PUBLIC -g -Wall -Wextra -pedantic)
