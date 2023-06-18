/* filesystem.h - header for file system manager 
 * NOTES:
 *      - inode represents a file & contains all info abt file (size, permissions, etc.)
 *      - file descriptor is index into data struct containing details of every opened file
 *      - fd created when file is opened by program
 *      - superblock: mounted file system
 *      - dentry: directory entry i.e. single path part
 *      - max file size:
 *      - max number of files: 4096/64 - 1 - 1 = 62
 *      - max file size: (4096-4)/4 * 4096 = 4MB\
 *      - number of data blocks: 8(4096-4)/32
 *      - open files represented by PCB (process control block or file array) & 8 can be open at a time
 *      - file array indexed by file descriptor
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "lib.h"
#include "types.h"

#define NUM_MAX_FILES       63                                  // 62 in reality because first is reserved for boot block
#define NUM_FILES           62 
#define MAX_OPEN_FILES      8                                  // max number of files that can be open at a time for a process

/* Directory Entry Struct: stores path for file object */
typedef struct dentry{
    int8_t file_name[BYTES_32B];
    int32_t file_type;
    int32_t inode_num;
    int8_t reserved[BYTES_24B];
} dentry_t;                                                   // 64B per entry

/* Boot Block Struct: stores info about filesystem object */
typedef struct bootblock{                                       // 4KB per block
    int32_t dir_count;                                          // size 4B = 32 bits
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[BYTES_52B];
    dentry_t direntries[NUM_MAX_FILES];
} bootblock_t;                                                 // 4KB per block

/* Inode Struct: stores info about file object (size, permissions, points to directory entry) */
typedef struct inode{
    int32_t length;
    int32_t data_block_num[BYTES_1KB - 1];
} inode_t;                                                     // 4KB per block

/* Data Block Struct: stores file data */
typedef struct datablock_t{
    uint8_t data[BYTES_4KB];
} datablock_t;                                                 // 4KB per block


/* File Descriptor/Operations/PCB Structs*/

typedef struct file_operations_table{
    int32_t (*read) (int32_t file_index, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t file_index, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* file_name);
    int32_t (*close)(int32_t file_index);
}file_operations_table_t;

typedef struct file_descriptor{
    file_operations_table_t* fop;   // jumptable for operations
    uint32_t inode_idx;             // inode num/index in inodes
    uint32_t file_position;         // where user is reading in file
    uint32_t flags;                 // if file is open or closed
    uint32_t file_type;
}file_descriptor_t;               // file descriptor

typedef struct pcb{
    uint32_t active; //1 if active, 0 if not
    int32_t pcb_pid;
    int32_t parent_pid;
    file_descriptor_t fda[MAX_OPEN_FILES];       // array that holds all open files; first two indices are reserved stdin & stdout
    uint32_t eip_val;
    uint32_t esp_val;
    uint32_t ebp_val;
    uint32_t tss_esp0;
    uint8_t args[BYTES_32B];

}pcb_t; //process control block

/* Global Pointers to Start of Filesystem Objects */
inode_t* inodes;
bootblock_t* boot_block;
datablock_t* data_blocks;
pcb_t * pcb_obj;
//dentry_t* dentries;

/* Functions to Manage Filesystem Components */
void filesystem_initialize(uint32_t start);
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint8_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Functions to Manage Processes/Open Files/Directories */
int32_t directory_read(int32_t file_index, void* buff, int32_t num_bytes);
int32_t directory_write(int32_t file_index, const void* buff, int32_t num_bytes);
int32_t directory_open(const uint8_t* fname);
int32_t directory_close(int32_t file_index);

/* Functions to Manage null Operations */
int32_t no_operation_read(int32_t file_index, void* buff, int32_t num_bytes);
int32_t no_operation_write(int32_t file_index, const void* buf, int32_t num_bytes);
int32_t no_operation_open(const uint8_t* fname);
int32_t no_operation_close(int32_t file_index);

/* Functions to Manage File Operations */
int32_t file_read(int32_t file_index, void* buff, int32_t num_bytes);

int32_t file_write(int32_t file_index, const void* buff, int32_t num_bytes);

int32_t file_open(const uint8_t* fname);

int32_t file_close(int32_t file_index);

#endif
