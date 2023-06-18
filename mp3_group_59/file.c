#include "file.h"

boot_block_t* boot_block;   //pointer to the boot block
inode_t* inodes;            //pointer to the first inode
data_block_t* data_blocks;  //pointer to the first data block
int file_index = 0;         //index of file, up to 63 files so 64 bit

/*
*init_file_system
*initilize the file system
*input: added file
*output:none
*side effect: file system memory might be changed
*/
void init_file_system(uint32_t file_add){

}



/*
*   read_dentry_by_name
*   Read the content of a dentry according to the given name
*   input: const uint8_t* fname -- the name of dentry need to be copied
*          dentry_t* dentry -- empty pointer ready for data
*   output: -1 for failure
*   side effect: the data pointed by dentry may be changed
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int index;
    dentry_t* loop_dentry;

    //check the length of the file name
    if ( (strlen((int8_t*)fname) > 32) || (strlen((int8_t*)fname) <= 0) ){
        return -1;
    }

    //loop the file name to see if there is a machting name
    for (index=0 ; index<boot_block.num_dir_entries ; index++){
        loop_dentry = &((boot_block->dentries)[index]);
        //compare the file name
        if (strncmp((int8_t*)fname,(int8_t*)loop_dentry.file_name,FILE_NAME_SPACE) == 0){
            //copy file type
            dentry->file_name = loop_dentry->file_name;
            //copy num_inodes
            dentry.num_inodes = loop_dentry.num_inodes;
            //copy reserved
            strncpy((uint32_t*)dentry.reserved,(uint32_t*)loop_dentry.reserved,6); //6 is reserved space

            return 0;
        }
    }
    //else, fail
    return -1;
}



/*
*   read_dentry_by_index
*   Read the content of a dentry according to the given index
*   input: uint8_t index -- the index of directory entry we want
*          dentry_t* dentry -- empty pointer where to put the content
*   output: -1 for failure
*   side effect: the data pointed by dentry may be changed
*/
int32_t read_dentry_by_index (uint8_t index, dentry_t* dentry){
    //check if the index is valid or not
    if (index > MAX_FILE_NUM || index > boot_block.num_dir_entries){
        return -1;
    }

    dentry_t* from = &((boot_block->dentries)[index]);
    
    //copy file name
    strncpy((uint8_t*)dentry->file_name,(uint8_t*)from->file_name,FILE_NAME_SPACE);
    //copy file type
    dentry->file_name = from->file_name;
    //copy num_inodes
    dentry.num_inodes = from.num_inodes;
    //copy reserved
    strncpy((uint32_t*)dentry.reserved,(uint32_t*)from.reserved,6); //6 is reserved space

    return 0;
}



/*
*   read_data
*   Read the content of a dentry with given index
*   input: uint32_t inode_index -- the index of inode we want
*          uint32_t offset -- which position of the file we want to start of our reading process
*          uint8_t* buf -- buffer for the data read from file
*          uint32_t length -- how many bytes we want to read
*   output: bytes of data we have successfully read
*   side effect: the data in buffer may be changed
*/
int32_t read_data(uint32_t inode_index, uint32_t offset, uint8_t* buf, uint32_t length)