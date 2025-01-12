project(BoxExample)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

add_executable(example main.cpp definitions.cpp)
target_include_directories(example PUBLIC ${PROJECT_SOURCE_DIR}/../)
