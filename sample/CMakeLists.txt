cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ping_sample)

zephyr_include_directories(include)

target_sources(app PRIVATE 
    src/main.c
    src/sidewalk_callbacks.c
    src/sidewalk_workitems.c
    src/sidewalk_globals.c
    src/queue.c
)
