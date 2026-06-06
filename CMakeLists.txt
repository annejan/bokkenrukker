cmake_minimum_required(VERSION 3.16)
project(goattrk2 LANGUAGES C CXX)

# Top-level wrapper so users can build the Qt frontend straight from the
# repo root:
#
#   cmake -S . -B build
#   cmake --build build -j
#
# Everything else lives under qt/.
add_subdirectory(qt)
