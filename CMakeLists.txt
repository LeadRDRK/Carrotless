cmake_minimum_required(VERSION 3.18.1)
set(MODULE_NAME main)
project(carrotless)

macro(SET_OPTION option value)
    set(${option} ${value} CACHE INTERNAL "" FORCE)
endmacro()

SET_OPTION(DOBBY_DEBUG OFF)
SET_OPTION(DOBBY_GENERATE_SHARED OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Dobby)

include_directories(${CMAKE_CURRENT_LIST_DIR}/rapidjson/include)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SQLiteCpp)

include_directories(include)

message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 23)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fexceptions -fno-rtti")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS} -O0")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_FTS5)
target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_RTREE)
target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_FTS3)
target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
target_compile_definitions(sqlite3 PUBLIC SQLITE_TEMP_STORE=3)
target_compile_definitions(sqlite3 PUBLIC SQLITE_USE_URI=1)

add_library(${MODULE_NAME} SHARED main.cpp hook.cpp il2cpp/il2cpp_symbols.cpp il2cpp_hook.cpp localify/localify.cpp logger/logger.cpp elf_util.cpp)
target_link_libraries(${MODULE_NAME} log dobby SQLiteCpp sqlite3)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.so"
            VERBATIM)
endif ()
