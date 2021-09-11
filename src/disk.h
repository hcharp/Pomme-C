#pragma once

#include "filesystem.h"

int disk_init();
void disk_deinit();
int disk_write_block(int blockno, struct block * block);
int disk_read_block(int blockno, struct block * block);
int disk_write_inode(int inodeno, struct inode * inode);
int disk_read_inode(int inodeno, struct inode * inode);
int disk_write_block_bitmap(char *);
int disk_write_inode_bitmap(char *);
