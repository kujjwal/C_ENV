cmake_minimum_required(VERSION 3.16)
project(C_Environment C)


set(AUTHOR "Ujjwal Krishnamurthi")
set(LICENSE "MIT")
set(CMAKE_C_STANDARD 99)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out/)
OPTION(DEBUG "add debug flags" OFF)

# Mac VM targets
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

# SQLite Clone target
file(GLOB SQL_SOURCES src/sqlite_clone/*.c)
add_executable(SQLite_Clone ${SQL_SOURCES})
# add_executable(Blockchain_Dummy src/blockchain_dummy/blockchain_dummy.h src/blockchain_dummy/blockchain_dummy.c)

# MQTT Broker target
if(DEBUG)
    message(STATUS "Configuring build for debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused -Werror -std=c99 -O3 -pedantic -luuid -ggdb -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -pg")
else(DEBUG)
    message(STATUS "Configuring build for production")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wunused -Werror -Wextra -std=c99 -O3 -pedantic -luuid")
endif()

file(GLOB MQTT_SOURCES src/mqtt_broker/*.c)
add_executable(mqtt_broker ${MQTT_SOURCES})