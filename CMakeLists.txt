cmake_minimum_required(VERSION 3.7)

project(ButtonTest C CXX)

add_executable(ButtonTest
    source/main.c
)

wut_create_rpx(ButtonTest)
