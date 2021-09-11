#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "disk.h"


struct hdd_file{
    FILE * file;
} hdd = {
    NULL, // no file in hdd at start
};


int disk_init()
{
    FILE * file;

    // "Open a binary file in append mode for reading or updating at the end of the file. fopen() creates the file if it does not exist."
    // https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-fopen-open-file
    // We use binary because we store raw data (not structured, not text, not images, it can be whatever)
    file = fopen("hdd", "wb+");

    if (!file) {
        fprintf(stderr, "unable to open file");
        return -1;
    }

    hdd.file = file;

    return 0;
}


void disk_deinit()
{
    fclose(hdd.file);
}


// blocks and inodes: see filestystem structure in filsystem.h
int disk_write_block(int blockno, struct block * block)
{
    size_t offset; // where the file pointer is

    if (blockno < 0 || blockno >= N_BLOCKS) {
        fprintf(stderr, "invalid block n°");
        return -1;
    }

    if (block ==  NULL) {
        fprintf(stderr, "block is NULL");
        return -1;
    }

    // find the offset of the block in bytes
    // see filesystem.h
    // offsetof(struct filesystem, blocks)) = n_inodes + n_blocks + sizeof(inode) x n_inodes
    offset = offsetof(struct filesystem, blocks) + blockno * sizeof(struct block);

    // set the file position to the given offset
    // see if error in fseek
    if (fseek(hdd.file, offset, SEEK_SET) != 0) // see stdio.h: "Seek from beginning of file."
        return -1;

    // write the block
    if (1 != fwrite(block, BLOCK_SIZE, 1, hdd.file)) // write block 1 time on BLOCK_SIZE bytes in the file hdd.file
        return -1;

    // force write to file (in case of buffering)
    fflush(hdd.file);

    return 0;
}


int disk_read_block(int blockno, struct block * block)
{
    size_t offset;

    if (blockno < 0 || blockno >= N_BLOCKS) {
        fprintf(stderr, "invalid block n°");
        return -1;
    }

    if (block ==  NULL) {
        fprintf(stderr, "block is NULL");
        return -1;
    }

    // find the offset of the block in bytes
    offset = offsetof(struct filesystem, blocks) + blockno * sizeof(struct block);

    // set the file position to the given offset
    if (fseek(hdd.file, offset, SEEK_SET) != 0)
        return -1;

    // read the block
    if (1 != fread(block, BLOCK_SIZE, 1, hdd.file))
        return -1;

    return 0;
}


int disk_write_inode(int inodeno, struct inode * inode)
{
    size_t offset;

    if (inodeno < 0 || inodeno >= N_INODE) {
        fprintf(stderr, "invalid invalid n°");
        return -1;
    }

    if (inode ==  NULL) {
        fprintf(stderr, "block is NULL");
        return -1;
    }

    // find the offset of the block in bytes
    offset = offsetof(struct filesystem, inodes) + inodeno * sizeof(struct inode);

    // set the file position to the given offset
    if (fseek(hdd.file, offset, SEEK_SET) != 0)
        return -1;

    // write inode
    if (1 != fwrite(inode, sizeof(struct inode), 1, hdd.file))
        return -1;

    // force write to file (in case of buffering)
    fflush(hdd.file);

    return 0;
}


int disk_read_inode(int inodeno, struct inode * inode)
{
    size_t offset;

    if (inodeno < 0 || inodeno >= N_INODE) {
        fprintf(stderr, "invalid invalid n°");
        return -1;
    }

    if (inode ==  NULL) {
        fprintf(stderr, "block is NULL");
        return -1;
    }

    // find the offset of the block in bytes
    offset = offsetof(struct filesystem, inodes) + inodeno * sizeof(struct inode);

    // set the file position to the given offset
    if (fseek(hdd.file, offset, SEEK_SET) != 0)
        return -1;

    // read inode
    if (1 != fread(inode, sizeof(struct inode), 1, hdd.file))
        return -1;

    return 0;
}


// write in free blocks table
int disk_write_block_bitmap(char * bitmap)
{
    size_t offset;

    // find the offset of the block in bytes
    offset = offsetof(struct filesystem, free_blocks);

    // set the file position to the given offset
    if (fseek(hdd.file, offset, SEEK_SET) != 0)
        return -1;

    // read inode
    if (1 != fwrite(bitmap, N_BLOCKS, 1, hdd.file))
        return -1;

    return 0;
}


int disk_write_inode_bitmap(char * bitmap)
{
    size_t offset;

    // find the offset of the block in bytes
    offset = offsetof(struct filesystem, free_inodes);

    // set the file position to the given offset
    if (fseek(hdd.file, offset, SEEK_SET) != 0)
        return -1;

    // read inode
    if (1 != fwrite(bitmap, N_INODE, 1, hdd.file))
        return -1;

    return 0;
}
