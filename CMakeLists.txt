cmake_minimum_required(VERSION 2.8) 
project(Log) 
add_executable(Log test.c log.c) 
set_target_properties(Log PROPERTIES COMPILE_FLAGS "-pthread" LINK_FLAGS "-pthread")