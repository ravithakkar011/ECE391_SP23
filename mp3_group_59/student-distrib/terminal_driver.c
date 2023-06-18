/* terminal_driver - Manages terminal and interacts with the keyboard.
 * NOTES: Read and write completed for checkpoint 2
 * 
 */
#include "terminal_driver.h"
int first_boot = 0;

/* void terminal_init();
 * Inputs: none
 * Return Value: 0
 * Function: Initializes terminal
*/
int32_t terminal_init(uint32_t terminal_id){
    
    //check if terminal id is valid
    if ((terminal_id > (NUM_TERMINALS - 1)) || (terminal_id < 0)){
        return -1;
    }
    //update current terminal id
    curr_terminal_id = terminal_id;
    
    //update terminal struct for the terminal
    terminals[terminal_id].active = 1;
    terminals[terminal_id].screen_X = 0;
    terminals[terminal_id].screen_Y = 0;

    //store current keyboard buffer into struct
    memcpy((uint8_t*)terminals[terminal_id].keyboard_buffer, (uint8_t*)keyboard_buffer, NUM_CHARS_KB);
 

    //find video memory address for terminal
    uint32_t terminal_vidmem_addr = VIDEO + ((terminal_id + 1) * BYTES_4KB);

    //clear video memory for the terminal
    clear_terminal_vidmem(terminal_vidmem_addr);

    terminals[terminal_id].video_mem_addr = terminal_vidmem_addr;

    send_eoi(KEYBOARD_IRQ_NUM);
    system_execute((uint8_t*)"shell");
    return 0;
}


//return 0 if success return -1 if fail
int32_t terminal_switch(uint32_t terminal_id){
    //check if terminal_id is the same as the current terminal id
    if(terminal_id == curr_terminal_id){
        return 0; // we dont need to do anything
    }

    terminals[curr_terminal_id].screen_X = get_screen_x();
    terminals[curr_terminal_id].screen_Y = get_screen_y();

    /*save current keyboard buffer to the current terminal struct*/
    memcpy((uint8_t*)terminals[curr_terminal_id].keyboard_buffer, (uint8_t*) keyboard_buffer, NUM_CHARS_KB);
    clear_keyboard_buff();
    /*save video memory page to the current terminal's video memory page*/
    memcpy((uint32_t*)terminals[curr_terminal_id].video_mem_addr, (uint32_t*)VIDEO, BYTES_4KB);
    clear();
    set_cursor(0,0);

    /*check if terminal to switch to has already been initialized or not*/
    if(terminals[terminal_id].active == 0){
        terminal_init(terminal_id);
        //printf("init: %d \n", terminal_id);
    }
    else{
        /*resore video memory for new terminal*/
        memcpy((uint32_t*)VIDEO, (uint32_t*)terminals[terminal_id].video_mem_addr, BYTES_4KB);

        //restore cursor for new terminal
        uint32_t new_screen_X = terminals[terminal_id].screen_X;
        uint32_t new_screen_Y = terminals[terminal_id].screen_Y;
        set_cursor(new_screen_X, new_screen_Y);
        
        memcpy((uint8_t*) keyboard_buffer, (uint8_t*) terminals[terminal_id].keyboard_buffer, NUM_CHARS_KB);

        curr_terminal_id = terminal_id;

    }
    return 0;
}





/* int terminal_open();
 * Inputs: none
 * Return Value: 0
 * Function: opens terminal
*/
int32_t terminal_open(const uint8_t* fname){
return 0;
}

/* int terminal_close();
 * Inputs: none
 * Return Value: 0
 * Function: closes terminal
*/
int32_t terminal_close(int32_t file_index){
return 0;
}





/* int32_t terminal_read();
 * Inputs: file_index (unused for CP2), argument buffer, number of bytes to be copied
 * Return Value: number of bytes copied
 * Function: copies keyboard buffer into the argument buffer after enter is pressed
*/
int32_t terminal_read(int32_t file_index, void* buf, int32_t nbytes){
    int32_t num_bytes_copied = 0; //keeps track of the number of bytes copied
    int i;
    //printf("nbytes in terminal_read: %d\n", nbytes);
    if ( (nbytes == 0) ||(buf == NULL) || (keyboard_buffer == NULL) ){ /*checking arguments to see if they aren't null or 0*/
        return nbytes;
    }
    sti(); //enable interrupts
    
    while(enter_pressed_flag != 1){ //enter flag to stop terminal read from executing
        i = 0;
    }
    //printf("num chars typed: %d\n", num_chars_typed);

    if(nbytes < NUM_CHARS_KB) { //if the number of bytes to be copied is less than the keyboard buffer size
        for(i = 0; i < num_chars_typed; i++) {
            //Fills in the user entered buffer with the resulting characters.
            ((char*) buf)[i] = keyboard_buffer[i];
            //If it is smaller than nbytes it finishes here.
            if(((char*)buf)[i] == '\0') {
                num_bytes_copied = i + 1;
                break;
            }
            //Makes sure that the last character in buf is a newline
            if(i == num_chars_typed){
                ((char*) buf)[i] = '\0';
                num_bytes_copied = i + 1;
                break;
            }
        }
    } 
    else { //this is when the number of bytes are higher, but still we are working with number of chars typed
        for(i = 0; i < num_chars_typed; i++) {
            ((char*) buf)[i] = keyboard_buffer[i];
            num_bytes_copied++;
        }
        ((char*)buf)[num_chars_typed] = '\0';

    }
    num_chars_typed = 0;  //Go back to the start of the char_buffer.
    enter_pressed_flag = 0;

    clear_keyboard_buff();
    sti(); //enable interrupts

    //printf("number of bytes copied: %d\n", num_bytes_copied);
    return num_bytes_copied;
}


/* int terminal_write();
 * Inputs: fd (unused for CP2), argument buffer, number of bytes to be printed to screen
 * Return Value: number of bytes printed
 * Function: prints contents of argument buffer to screen
*/
int terminal_write(int32_t file_index, const void* buf, int nbytes){
    int i;
    //char* character; 
    char current_char;
    if ((nbytes == 0)||(buf == NULL)) // evaluates arguments
    {
        return 0; // if they are not valid returns 0
    }

    if(nbytes <= NUM_COLS){
        uint8_t temp_buffer[NUM_COLS]; //initialize a temp buffer to extract keyboard buffer values
        
        for (i = 0; i < NUM_COLS; i++){
            temp_buffer[i] = '\0';
        }

        temp_buffer[NUM_COLS - 1] = '\n';

        int * chk_memret; //used to check return value for memcpy

        chk_memret = memcpy(temp_buffer, buf, nbytes);
        if(chk_memret == 0){
            //printf("Memcpy error in terminal write\n");
            return 0;
        }

        for (i = 0; i < NUM_COLS; i++){
            current_char = ((char*)temp_buffer)[i];
            if(current_char == 0){
                return 0;
            }
            putc(current_char);
        }
    }
    else{
        for (i = 0; i < nbytes; i++){
            current_char = ((char*)buf)[i];
            if(current_char == 0){
                return 0;
            }
            putc(current_char);
        }
    }
    //putc('\0');
    //printf("%s \n", keyboard_buffer);
    return nbytes;
}




