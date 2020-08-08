cmake_minimum_required(VERSION 3.16)
project(C_Environment C)

set(CMAKE_C_STANDARD 99)

add_executable(MAC_VM_simple src/Simple_VM/mac_simple/mac_simple.c)
add_custom_target(make_mac_vm_simple
        COMMAND make run
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Simple_VM/mac_simple
        )

add_executable(MAC_VM src/Simple_VM/mac/mac.c)
add_custom_target(make_mac_vm
        COMMAND make run
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/Simple_VM/mac
        )

add_executable(SQLite_Clone src/sqlite_clone/main.c src/sqlite_clone/compiler.c src/sqlite_clone/inputstream.c src/sqlite_clone/database.c src/sqlite_clone/btree.c)