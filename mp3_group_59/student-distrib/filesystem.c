/* filesystem.c - manages entire file system 
 * Functions: filesystem_initialize, read_dentry_by_index, read_dentry_by_name, read_data, read_file
    * 			  file_open, file_close, file_read, file_write, dir_open, dir_close, dir_read, dir_write
    * NOTES: 
 */ 

#include "filesystem.h"



/*
*   FUNCTION: filesystem_initialize
*   DESCRIPTION: Fill memory with initialized objects for filesystem
*   INPUTS: 
*           uint32_t* start -- starting address of filesystem memory image
*   OUTPUTS: none
*   SIDE EFFECTS: file opened
*/
void filesystem_initialize(uint32_t start){
    boot_block = (bootblock_t*)start;                                                     // put boot block at start of filesystem img
    inodes = (inode_t*)(boot_block + 1);                                    // add 1 for boot block
    data_blocks = (datablock_t*)(inodes + boot_block->inode_count);         // set up addr for data blocks

    // printf("Filesystem initialized \n");
}

/*
*   FUNCTION: read_dentry_by_index
*   DESCRIPTION: Scans through directory entries in the boot block to find the 
*   file index (index is not inode but index into boot block). Fills dentry with file name, file type, & inode and return 0.
*   INPUTS: 
*           uint8_t index -- index of dentry in boot block
*           dentry_t* dentry -- buffer to fill with read data
*   OUTPUTS: 0 for success; -1 for fail (non-existent file or invalid index)
*   SIDE EFFECTS: file opened
*/
int32_t read_dentry_by_index(uint8_t index, dentry_t* dentry){
    if (index > boot_block->dir_count || NUM_MAX_FILES < index){
        return -1;
    }

    /* fill each of the parameters */
    int i;
    for(i=0; i<BITS_32; i++){
        dentry->file_name[i] = (&((boot_block->direntries)[index]))->file_name[i];
    }
    //strncpy((char*)dentry->file_name, (char*)(&((boot_block->direntries)[index]))->file_name, BITS_32);
    dentry->file_type = (&((boot_block->direntries)[index]))->file_type;
    dentry->inode_num = (&((boot_block->direntries)[index]))->inode_num;
    //printf("%d\n", (int)dentry->inode_num);

    // for(i=0; i < num_indices; i++){
       
    // }
    
    return 0;
}

/*
*   FUNCTION: read_dentry_by_name
*   DESCRIPTION: Scans through directory entries in the boot block to find the file name
*   INPUTS: 
*           const uint8_t* fname -- name of read dentry
*           dentry_t* dentry -- buffer to fill with read data
*   OUTPUTS: 0 for success; -1 for fail (non-existent file or invalid index)
*   SIDE EFFECTS: file opened
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    // scan directory entries in boot block
    uint8_t i;
    int num_indices = boot_block->dir_count; //sizeof(boot_block.direntries);
    // call read_dentry_by_index() which populates dentry parameter (file name, file type, inode num)
    for(i=0; i < num_indices; i++){
        if(!strncmp((char*)fname, ((&((boot_block->direntries)[i]))->file_name), BITS_32)){                        // size of filename is 32
            read_dentry_by_index(i, dentry);
            return 0;
        }
        else{
            continue;
        }
    }
    return -1;
}

/*
*   FUNCTION: read_data
*   DESCRIPTION: Reads/copies data from a file at inode, starting from offset into file, & of size length
*   INPUTS: 
*           uint32_t inode -- file index
*           uint32_t offset -- location of read start within file
*           uint8_t* buf -- buffer to to fill with read data
*           uint32_t length -- length in bytes of how much data to read
*   OUTPUTS: number of bytes read/copied
*   SIDE EFFECTS: file opened
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    inode_t curr_inode = inodes[inode];
    uint32_t curr_data_block_array_index;
    uint32_t curr_data_block_index;
    //printf("Read Data Reached!\n");
    // inode range check
    // if(inode <= -1 || inode > boot_block->inode_count-1){
    if(inode > boot_block->inode_count-1){ //if the target inode index doesn't exist
        printf("read_data: Inode out of range \n");
        return -1;
    }

    if(offset >= curr_inode.length){                                // has the end of the file been reached by offset -> return 0 (check docs)
        return 0;
    }
    
    //if the offset(start) is valid, check if the offset+length(end) is valid
    // if(offset+length > curr_inode.length){
    //     printf("read_data: Inode out of range1 \n");
    //     return -1;
    // }
    
    //uint8_t temp_buf[NUM_MAX_FILES]; //initialize a temp buffer to write bits into
    
    uint32_t current_byte_index; //the byte that we are currently copying
    uint8_t current_byte; //holds current byte to be written into buffer
    uint32_t num_bytes_copied = 0; //counts the number of bytes copied
    uint32_t current_byte_index_within_block; // finds the current index of a byte within a data block
    int i = 0;
    //write each individual byte into temp buffer
    for(current_byte_index = offset; current_byte_index < (offset + length); current_byte_index++){
        
        if(current_byte_index == curr_inode.length){
            //printf("current byte index = %d, file length = %d\n", current_byte_index, curr_inode.length);
            return current_byte_index - offset;
        }

        //printf("%d \n", current_byte_index);
        curr_data_block_array_index = current_byte_index / BYTES_4KB; //find the current data block index for array

        curr_data_block_index = inodes[inode].data_block_num[curr_data_block_array_index]; //find the current data block index within filesystem

        current_byte_index_within_block = current_byte_index % BYTES_4KB;
        //printf("%d \n", current_byte_index_within_block);
        current_byte = data_blocks[curr_data_block_index].data[current_byte_index_within_block];
        //printf("%d \n", current_byte);
        //putc(current_byte);
        //putc("\n");
        
        //adds current byte to temp buffer
        //temp_buf[num_bytes_copied] = current_byte;
        buf[i] = current_byte;
        i++;
        num_bytes_copied++;
    }

    //memcpy(buf, temp_buf, length);
    return num_bytes_copied;
}

/*
*   FUNCTION: directory_read
*   DESCRIPTION: Reads entire directory at index into boot block
*   INPUTS: 
*           int byte_count -- number of bytes of data to read/copy into buffer
*           uint8_t* buf -- buffer that data gets read/copied into
*           int index -- indedx into boot block
*   OUTPUTS: bytes read for success; -1 for fail
*   SIDE EFFECTS: file opened
*/
int32_t directory_read(int32_t file_index, void* buff, int32_t num_bytes){
    // uses read_data()
    //printf("directory_read: file index: %d \n", file_index);
    // clear buff?
    dentry_t dentry;
    //uint32_t inode_index = pcb_obj->fda[file_index].inode_idx;
    //int32_t file_size = inodes[inode_index].length;
    if(read_dentry_by_index(pcb_obj->fda[file_index].file_position, &dentry) == -1){
        //printf("Error in line 173! \n");
        return -1;
    }
    
    int8_t* dest_ptr;
    //int8_t filename_length = strlen((int8_t*)&dentry.file_name);
    dest_ptr = strncpy((int8_t*)buff, (int8_t*)&dentry.file_name, BYTES_32B); //(uint32_t)filename_length
    if (dest_ptr == NULL){
        //printf("Strncpy error!\n");
        return -1;
    }
    pcb_obj->fda[file_index].file_position = pcb_obj->fda[file_index].file_position + 1;
    //printf("reached \n");
    int strlen_ret = strlen((int8_t*)buff);
    // printf("strlen buff =  %d\n", strlen_ret);
    // printf("strlen filename=  %d\n", filename_length);
    /* case for when filename is larger than allowed */
    if(strlen_ret > BYTES_32B){
        // printf("Error: Length of file longer than 32 \n");
        return BYTES_32B;
    }
    else{
        return strlen_ret;
    }
    //read_data();
}

/*
*   FUNCTION: write
*   DESCRIPTION: none
*   INPUTS: none 
*   OUTPUTS: -1
*   SIDE EFFECTS: none
*/
int32_t directory_write(int32_t file_index, const void* buff, int32_t num_bytes){
    return -1;
}

/*
*   FUNCTION: open
*   DESCRIPTION: Initializes filesystem temporary structures/opens a directory file
*   INPUTS: 
*           int32_t index -- 
*   OUTPUTS: 0 for success; -1 for fail
*   SIDE EFFECTS: file/process opened & filesystem mounted
*/
int32_t directory_open(const uint8_t* fname){
    // open directory file ?
    return 0;
}

/*
*   FUNCTION: close
*   DESCRIPTION: Undo filesystem temporary structures
*   INPUTS: 
*           int32_t index -- 
*   OUTPUTS: 0 for success; -1 for fail
*   SIDE EFFECTS: file opened
*/
int32_t directory_close(int32_t file_index){
    // should do nothing
    return 0;
}

/* Null operation file table functions (all return -1 bc no operation) */
int32_t no_operation_read(int32_t file_index, void* buff, int32_t num_bytes){
    return -1;
}
int32_t no_operation_write(int32_t file_index, const void* buf, int32_t num_bytes){
    return -1;
}
int32_t no_operation_open(const uint8_t* fname){
    return -1;
}
int32_t no_operation_close(int32_t file_index){
    return -1;
}

/*Function: file_read( int32_t file_index, void* buff, int32_t num_bytes)
 *Description: reads data from a file into a buffer
 *Input: file_index -- index of file in file descriptor array
 *       buff -- buffer to read data into
 *       num_bytes -- number of bytes to read
 *Output: number of bytes read
 *Side Effects: none
*/
int32_t file_read(int32_t file_index, void* buff, int32_t num_bytes){        
    int32_t num_bytes_read;
    if(num_bytes <= 0){ //check if number of bytes is valid
        printf("file_read: Number of bytes is out of range \n");
        return -1;}
    
    num_bytes_read = read_data(pcb_obj->fda[file_index].inode_idx, pcb_obj->fda[file_index].file_position, buff, num_bytes);         // pass appropriate file data from file found from file_index into file descriptor array (inode, offset, buf, length)
    if(num_bytes_read == -1){
        return -1;
    }
    pcb_obj->fda[file_index].file_position += num_bytes_read; //update file position
    return num_bytes_read;
}


int32_t file_write(int32_t file_index, const void* buff, int32_t num_bytes){
    return -1;
}

int32_t file_open(const uint8_t* fname){
    return 0;
}

int32_t file_close(int32_t file_index){
    return 0;
}
