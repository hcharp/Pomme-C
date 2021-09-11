#pragma once

#include <stdio.h>

#define BLOCK_SIZE   1024
#define N_BLOCKS     30
#define N_INODE      15
#define DIR_NAME_LEN 20
#define MAX_DIRENT   BLOCK_SIZE / (DIR_NAME_LEN + 4) // sizeof(int) = 4


enum filetype { BIN, DIR };


struct inode
{
    char mode[8];
    enum filetype type;
    int  blocks[N_BLOCKS]; // at most N_BLOCKS, no indirection
};


struct block
{
    char data[BLOCK_SIZE];
};


// how a filesystem is structured
struct filesystem
{
    char   free_inodes[N_INODE]; // tab[i] = 0 if free inode, tab[i] = 1 if not
    char   free_blocks[N_BLOCKS]; // tab that starts after the free inodes tab
    struct inode inodes[N_INODE]; // position = N_INODE + N_BLOCKS
    struct block * blocks; // not a tab because it uses the RAM and it will use one byte of RAM for one byte of file
};


// entry in a directory is a file or a subdirectory tuple inode number - name
struct dirent
{
    int  inode_nb;
    char name[DIR_NAME_LEN];
};


// for now we stored directory entries inside one block
// so only BLOCK_SIZE/sizeof(dirent) = 1024/(20+4) = 42 entries
// can be stored in a block,
// TODO: dynamic directory with implementation of opendir, ...
struct dir
{
    int    n_entry; // number of entries
    struct dirent entries[MAX_DIRENT];
};


// to compile: 
// init filesystem
int fs_init();
// format filesystem
int fs_format();
// create new directory
int fs_mkdir(char * name, int parent_inodeno);
// create new file
int fs_create(char * name, int parent_inodeno);
// remove a directory or file
int fs_rm(char * name, int parent_inodeno);
