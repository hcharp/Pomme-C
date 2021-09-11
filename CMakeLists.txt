cmake_minimum_required (VERSION 3.9)
project (Pomme-C VERSION 0.1.0 LANGUAGES C)

# build

add_executable(Pomme-C) # test-filesystem is created with the command make

# in Pomme-C, we use these sources:
target_sources(Pomme-C PRIVATE
    src/disk.c
    src/filesystem.c
    src/shell.c
    )

add_executable(Screen) # Screen is created with the command make

# in Screen, we use this source:
target_sources(Screen PRIVATE
    screen/screen.c
    )