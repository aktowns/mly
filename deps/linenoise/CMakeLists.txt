cmake_minimum_required(VERSION 3.8)
project(linenoise)

set(CMAKE_C_STANDARD 11)

add_library(linenoise STATIC linenoise.c linenoise.h)