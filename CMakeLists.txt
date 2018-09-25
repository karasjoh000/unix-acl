cmake_minimum_required(VERSION 3.10)
project (cs483_p1)
add_executable (main src/acl.c src/get.c src/getregex.c src/debug.c src/main.c)
include_directories(src/include)