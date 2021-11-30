cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

add_executable(RayTracing aarect.h box.h bvh.h constant_medium.h hittable.h hittable_list.h main.cc material.h moving_sphere.h sphere.h 1.jpg)

add_definitions("-Wall -g")

