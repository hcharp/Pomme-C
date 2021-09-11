#include <string.h>

#include "filesystem.h"
#include "disk.h"

struct filesystem fs;

// search for a free inode
// @return first free inode number
static int search_free_inode(struct filesystem * f)
{
    int i;
    for (i = 0; i < N_INODE; i++) {
        if (!f->free_inodes[i])
            return i;
    }
    return -1;
}

// search for a free block
// @return first free block number
static int search_free_block(struct filesystem * f)
{
    int i;
    for (i = 0; i < N_BLOCKS; i++) {
        if (!f->free_blocks[i])
            return i;
    }
    return -1;
}

static int read_dir(int blockno, struct dir * d)
{
    struct block b;

    if (disk_read_block(blockno, &b) < 0) {
        fprintf(stderr, "unable to read disk");
        return -1;
    }

    // cast a directory struct on the block
    //d = (struct dir *)&b;
    memcpy(d, &b, sizeof(struct dir));

    return 0;
}

static int write_dir(int blockno, struct dir * d)
{
    if (disk_write_block(blockno, (struct block *)d) < 0) {
        fprintf(stderr, "unable to write on disk");
        return -1;
    }
    return 0;
}

/**
 * @brief find an entry in a directory
 * @name name of the entry
 * @d pointer to directory
 * @return index of entry or -1 if entry no found
 */
static int find_entry(char * name, struct dir * d)
{       
    int i;
    for (i = 0; i < d->n_entry; i++) {
        if (strcmp(d->entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int display_entries(char * name, struct dir * d)
{       
    int i;
    for (i = 0; i < d->n_entry; i++) {
        printf("element : %s\n", d->entries[i].name);
    }
}

int fs_init()
{
    // already in disk_init() (see disk.c)
    // FILE *hdd;
    // hdd = fopen("hard_drive", "wb+");
    // if (!hdd) {
    //     return -1;
    // }

    //fs_format();

    // create root @ inode 0, data stored in block 0
    fs.inodes[0].type = DIR;
    fs.inodes[0].blocks[0] = 0;

    // block 0 data is a directory structure
    struct dir directory;
    directory.n_entry = 2;
    directory.entries[0].inode_nb = 0;
    strcpy(directory.entries[0].name, ".");
    // parent is itself for root
    directory.entries[1].inode_nb = 0;
    strcpy(directory.entries[1].name, "..");

    // set inode 0 and block 0 to occupied
    fs.free_inodes[0] = 1;
    fs.free_blocks[0] = 1;

    // write changes to disk:
    if (write_dir(0, &directory) < 0                ||
        disk_write_inode(0, &fs.inodes[0]) < 0      ||
        disk_write_inode_bitmap(fs.free_inodes) < 0 ||
        disk_write_block_bitmap(fs.free_blocks) < 0) {
        fprintf(stderr, "unable to write to disk\n");
        return -1;
    }

    return 0;
}


int fs_mount()
{
    return 0;
}


int fs_umount()
{
    return 0;
}


// format: everything is free (inodes and blocks)
int fs_format()
{
    int i, j;

    // reset inodes
    for (i = 0; i < N_INODE; i++) {
        fs.inodes[i].type = 0;
        for (j = 0; j < N_BLOCKS; j++) fs.inodes[i].blocks[j]=-1;
    }

    // free inodes and blocks
    memset(fs.free_inodes, 0, sizeof(fs.free_inodes));
    memset(fs.free_blocks, 0, sizeof(fs.free_blocks));

    return 0;
}

// creating new dir
// -> check if there is an available inode and get its nb
// -> check if there is an available block and get its nb
// -> add entry to directory (name + inode_nb)
// -> create new directory struct and write it to the block
// -> update the free_inodes & free_blocks tables
// -> write changes on disk
int fs_mkdir(char * name, int parent_inodeno)
{
    int inodeno, blockno, parent_blockno;
    struct dir parent_dir;

    inodeno = search_free_inode(&fs);
    if (inodeno < 0) {
        // no free inodes have been found, report an error
        fprintf(stderr, "no free inodes");
        return -1;
    }

    blockno = search_free_block(&fs);
    if (blockno < 0) {
        // no free blocks have been found, report an error
        fprintf(stderr, "no free blocks");
        return -1;
    }

    parent_blockno = fs.inodes[parent_inodeno].blocks[0];
    if (read_dir(parent_blockno, &parent_dir) < 0) {
        fprintf(stderr, "unable to read dir");
        return -1;
    }
    // check if we can add a new entry
    if (parent_dir.n_entry+1 >= MAX_DIRENT) {
        fprintf(stderr, "number max of entry reached");
        return -1;
    }
    // check if the lenght of the name is ok
    if (strlen(name) > DIR_NAME_LEN) {
        fprintf(stderr, "name too big");
        return -1;
    }
    // create new entry and append it at the end of the parent directory
    parent_dir.entries[parent_dir.n_entry].inode_nb = inodeno;
    strcpy(parent_dir.entries[parent_dir.n_entry].name, name);
    parent_dir.n_entry++;

    struct inode inode = {.type = DIR, .blocks = {-1}};
    inode.blocks[0] = blockno;
    fs.inodes[inodeno] = inode;

    struct dir directory;
    directory.n_entry = 2;
    directory.entries[0].inode_nb = inodeno;
    strcpy(directory.entries[0].name, ".");
    // parent is itself for root
    directory.entries[1].inode_nb = parent_inodeno;
    strcpy(directory.entries[0].name, "..");

    fs.free_inodes[inodeno] = 1;
    fs.free_blocks[blockno] = 1;

    // write changes to disk:
    if (write_dir(parent_blockno, &parent_dir) < 0          ||
        write_dir(blockno, &directory) < 0                 ||
        disk_write_inode(inodeno, &fs.inodes[inodeno]) < 0 ||
        disk_write_inode_bitmap(fs.free_inodes) < 0 ||
        disk_write_block_bitmap(fs.free_blocks) < 0) {
        fprintf(stderr, "unable to write to disk");
        return -1;
    }


    return 0;
}

// creating new file
// -> check if there is an available inode and get its nb
// -> add entry to directory (name + inode_nb)
// -> create new inode for file
// -> update the free_inodes & free_blocks tables
// -> write changes on disk
int fs_create(char * name, int parent_inodeno)
{
    int inodeno, parent_blockno;
    struct dir parent_dir;

    inodeno = search_free_inode(&fs);
    if (inodeno < 0) {
        // no free inodes have been found, report an error
        fprintf(stderr, "no free inodes");
        return -1;
    }
    parent_blockno = fs.inodes[parent_inodeno].blocks[0];
    if (read_dir(parent_blockno, &parent_dir) < 0) {
        fprintf(stderr, "unable to read dir");
        return -1;
    }

    // check if we can add a new entry
    if (parent_dir.n_entry+1 >= MAX_DIRENT) {
        fprintf(stderr, "number max of entry reached");
        return -1;
    }
    // check if the lenght of the name is ok
    if (strlen(name) > DIR_NAME_LEN) {
        fprintf(stderr, "name too big");
        return -1;
    }
    // create new entry and append it at the end of the parent directory
    parent_dir.entries[parent_dir.n_entry].inode_nb = inodeno;
    strcpy(parent_dir.entries[parent_dir.n_entry].name, name);
    parent_dir.n_entry++;

    struct inode inode = {.type = BIN, .blocks = {-1}};
    fs.inodes[inodeno] = inode;
    fs.free_inodes[inodeno] = 1;

    // write changes to disk:
    if (write_dir(parent_blockno, &parent_dir) < 0          ||
        disk_write_inode(inodeno, &fs.inodes[inodeno]) < 0 ||
        disk_write_inode_bitmap(fs.free_inodes) < 0) {
        fprintf(stderr, "unable to write to disk");
        return -1;
    }

    return 0;
}

// removing a file (normal file  or directory)
// -> check if the file exist in the parent directory
// -> update the free_inodes & free_blocks tables
// -> remove the entry in the parent directory
// -> write changes on disk
int fs_rm(char * name, int parent_inodeno)
{
    int entry_idx, inodeno, parent_blockno, blockno,  i;
    struct dir parent_dir;

    parent_blockno = fs.inodes[parent_inodeno].blocks[0];
    if (read_dir(parent_blockno, &parent_dir) < 0) {
        fprintf(stderr, "unable to read dir");
        return -1;
    }
    entry_idx = find_entry(name, &parent_dir);
    if (entry_idx < 0) {
        fprintf(stderr, "file not found");
        return -1;
    }

    // free the blocks associated with the file
    inodeno = parent_dir.entries[entry_idx].inode_nb;
    for (i = 0; i < N_BLOCKS; i++) {
        blockno = fs.inodes[inodeno].blocks[i];
        if (blockno > 0) {
            fs.free_blocks[blockno] = 0;
        }
    }

    // free the inode of the file
    fs.free_inodes[inodeno] = 0;

    // move all the entries to account for this deletion
    for (i = entry_idx; i < parent_dir.n_entry - 1; i++) {
        parent_dir.entries[i] = parent_dir.entries[i+1];
    }
    parent_dir.n_entry--;

    if (write_dir(parent_blockno, &parent_dir) < 0   ||
        disk_write_block_bitmap(fs.free_blocks) < 0 ||
        disk_write_inode_bitmap(fs.free_inodes) < 0) {
        fprintf(stderr, "unable to write to disk");
        return -1;
    }

    return 0;
}

char * fs_ls(int parent_inodeno){
    int entry_idx, inodeno, parent_blockno, blockno,  i;
    struct dir parent_dir;

    parent_blockno = fs.inodes[parent_inodeno].blocks[0];
    if (read_dir(parent_blockno, &parent_dir) < 0) {
        fprintf(stderr, "unable to read dir\n");
        return NULL;
    }
    static char res[BLOCK_SIZE];
    strcat(res, "INODE - NAME\n");

    for (int i = 0; i < parent_dir.n_entry; i++){
        char tmp[1000];
        sprintf(tmp, "%d - %s\n", parent_dir.entries[i].inode_nb, parent_dir.entries[i].name);
        strcat(res, tmp);
    }
    return res;
}