cmake_minimum_required (VERSION 3.9)
project (Pomme-C VERSION 0.1.0 LANGUAGES C)

# build

add_executable(test-filesystem) # test-filesystem is created with the command make

# in test-filesystem, we use these sources:
target_sources(test-filesystem PRIVATE
    file_tree/main.c
    file_tree/disk.c
    file_tree/filesystem.c
    )

