cmake_minimum_required(VERSION 3.8)
project(mly)

add_subdirectory(deps/linenoise)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} "-Werror -Wall -std=gnu11 -fPIC -fblocks -fstack-protector -g -Werror=array-bounds -fsanitize=address,undefined -fno-omit-frame-pointer -fno-common")

find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GC REQUIRED bdw-gc)

set(LEMON_SOURCE_FILE ${CMAKE_SOURCE_DIR}/deps/lemon/lemon.c)
set(LEMON_TEMPLATE ${CMAKE_SOURCE_DIR}/deps/lemon/lempar.c.tmpl)
find_package(LEMON 1.0 REQUIRED)
lemon_target(parser ${CMAKE_SOURCE_DIR}/parser.y ${CMAKE_BINARY_DIR})

find_package(RAGEL 6.10 REQUIRED)
ragel_target(lexer lexer.rl ${CMAKE_CURRENT_BINARY_DIR}/lexer.c DEPENDS ${LEMON_parser_HEADER_OUTPUT})

find_library(blocks NAMES BlocksRuntime libBlocksRuntime PATHS /usr/lib)

set(SOURCE_FILES src/main.c src/syntax/token.c src/syntax/token.c src/include/token.h src/ast/ast.c src/include/ast.h src/types/type.c src/include/type.h src/sema/infer.c src/include/infer.h src/types/env.c src/include/env.h src/utils/g_list_utils.c src/include/g_list_utils.h src/ast/ast_pp.c src/include/ast_pp.h src/repl.c src/ast/xmlnode.c src/include/xmlnode.h src/utils/g_hash_table_utils.c src/include/g_hash_table_utils.h src/sema/unify.c src/include/unify.h)

add_executable(mly ${LEMON_parser_OUTPUTS} ${RAGEL_lexer_OUTPUTS} ${SOURCE_FILES})

target_compile_options(mly PUBLIC ${GLIB_CFLAGS_OTHER} ${GC_FLAGS_OTHER} -DG_LOG_USE_STRUCTURED=1 -fsanitize=address,undefined)
target_link_libraries(mly ${blocks} ${GLIB_LIBRARIES} ${GC_LIBRARIES} linenoise)
target_include_directories(mly PUBLIC ${GLIB_INCLUDE_DIRS} ${GC_INCLUDE_DIRS})
target_include_directories(mly PRIVATE src/include)
target_include_directories(mly PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(mly PRIVATE deps/linenoise)
