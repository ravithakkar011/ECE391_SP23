
.data					/* section declaration */

        /* Useful offset constants for accessing members of a 
        struct mp1_blink_struct structure*/
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

/* Pointer to head of list (initialized to NULL)*/
mp1_list_head:
        .long   0

.text					/* section declaration */

/* Export the function symbol names*/

.global mp1_rtc_tasklet
.global mp1_ioctl

/*creates jump table for mp1_ioctl*/
jumptable: 
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync

/* void mp1_poke(void);

 Interface: Register-based arguments (not C-style)
    Inputs: %cl  - The byte you wish to write
            %eax - Offset from the start of video memory that you wish
                   to write to
   Outputs: Text-mode video screen is written to at location %eax with
            the byte in %cl
 Registers: Clobbers EDX */
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret
/* void mp1_rtc_tasklet (unsigned long);

 Interface: C-style calling convention
    Inputs: mp1_list_head list (global)
   Outputs: Returns when mp1_list_head is traversed
 Registers: Clobbers %eax,%ecx,%edx(through mp1_poke) 
*/
mp1_rtc_tasklet:
        /*Creating Stack Frame*/
	pushl   %ebp
        movl    %esp, %ebp 
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        /*Examine head pointer*/
        movl    mp1_list_head, %ecx     /*Stores head pointer in %ecx*/
        cmpl    $0, %ecx                /*Checks if %ecx is pointing to null */
        je      tasklet_teardown        /*If so, begins stack teardown */
        jmp     tasklet_countdown_status /*Jumps to decrement countdown and status tasks*/

tasklet_list_traverse:
        movl    NEXT(%ecx), %ecx        /*%ecx points to head of next struct in the list*/
        cmpl    $0, %ecx                /*Check if ecx is pointing to a null pointer or not*/
        je      tasklet_teardown        /*If ecx points to null, tear down the stack*/
        
tasklet_countdown_status:
        /*decrement countdown */
        xorl    %eax, %eax              
        movw    COUNTDOWN(%ecx), %ax                                                                 
        decw    %ax                     /*Decrements the value stored in the countdown */
        movw    %ax, COUNTDOWN(%ecx)    /*Rewrites countdown with decremented value*/
        cmpw    $0, COUNTDOWN(%ecx)     
        jne     tasklet_list_traverse   /*If countdown != 0, continue traversal*/

        /*If countdown is 0, status will be examined. */
        xorl    %eax, %eax              
        movw    STATUS(%ecx), %ax
        movl    %ecx, %ebx              /*Moves pointer to struct to %ebx to protect pointer from being overwritten*/
        cmpw    $0, %ax                 
        jl      tasklet_status_error    /*Makes sure status is either 0 or 1 */
        je      tasklet_on_char         /*and jumps to invert status and character being printed */        
        cmpw    $1, %ax
        jg      tasklet_status_error
        je      tasklet_off_char

        /*Moving opposite character into %cl and inverting status value */
tasklet_on_char:
        xorb    %cl, %cl
        movb    ON_CHAR(%ebx), %cl      /*Moves opposite character value into %cl */
        movw    $1, STATUS(%ebx)        /*Inverts status value */
        movw    ON_LENGTH(%ebx), %ax
        movw    %ax, COUNTDOWN(%ebx)    /*Updates countdown */
        jmp     tasklet_poke_call

tasklet_off_char:
        xorb    %cl, %cl
        movb    OFF_CHAR(%ebx), %cl 
        movw    $0, STATUS(%ebx)        
        movw    OFF_LENGTH(%ebx), %ax
        movw    %ax, COUNTDOWN(%ebx) 
        jmp     tasklet_poke_call

tasklet_poke_call:
        /*Calling mp1_poke*/
        xorl    %edx, %edx 
        xorl    %eax, %eax              /*Clears %eax before use by mp1_poke */
        movw    LOCATION(%ebx), %ax     /*Moves value stored in location to %ax */ 
        shll    $1, %eax                /*Divides location value by 2 */
        call    mp1_poke
        movl    %ebx, %ecx              /*Moves pointer back to %ecx after using %cl */
        jmp     tasklet_list_traverse

tasklet_status_error:
tasklet_teardown:
        /*Stack teardown*/
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
        ret


/* int mp1_ioctl (unsigned long arg, unsigned long cmd);

 Interface: Assembly Linkage / Stack Based Arguments
    Inputs: command value in stack
   Outputs: Returns -1 if error occurs
 Registers: Clobbers %ecx, %eax
*/
mp1_ioctl:
        /*Retrieving and Initializing Values*/
        movl    8(%esp), %ecx           /*%ecx <- cmd */
        xorl    %eax, %eax 

        /*Checking cmd value for invalid numbers*/
        cmpl    $3, %ecx                /*Compares the cmd value with 3*/
        jg      ERROR_CMD_VALUE         /*If cmd > 3, return -1*/
        cmpl    $0, %ecx                /*Compares the cmd value with 0*/
        jl      ERROR_CMD_VALUE         /*If cmd < 0, return -1*/
        
        /*Jumptable*/
        jmp     *jumptable(,%ecx,4)     

ERROR_CMD_VALUE:
        dec     %eax                    /*Decrements value in eax to -1*/
        ret
/* int mp1_ioctl_add(unsigned long arg)

 Interface: C-style calling convention (arguments retrieved from stack)
    Inputs: arg (pointer to memory location in userspace)
   Outputs: returns 0 for success and -1 for error
 Registers: Clobbers %eax, %ecx, %edx
*/
mp1_ioctl_add:

	
/* register assignments:
        ebx: top of the address when mallocing, holds top of struct
        ecx: top of struct from arg
        eax: temp register, return register
        e[dx]: holds struct size
*/
        /*Creating Stack Frame*/
	pushl   %ebp
        movl    %esp, %ebp 
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        /*Initializing values*/
        xorl    %edx, %edx
       
        /*Malloc*/
        pushl   $STRUCT_SIZE            /*Pushing struct size to be used as a parameter by mp1_malloc*/
        call    mp1_malloc              /*Top address returned through %eax */
        addl    $4, %esp                /*Moving top of stack pointer in correspondance with pushl*/
        movl    %eax, %ebx              /*Move pointer to %ebx*/

        cmpl    $0, %ebx                /*Error check, see if malloc returned null */
        je      add_error               /*If %ebx is pointing to null, jump to add_error*/

        /*Obtaining Argument*/
        movl    8(%ebp), %ecx           /*arg -> %ecx */
        cmpl    $0, %ecx                /*Error check to see if arg is pointing to null */
        je      add_error               

        /*Copying Struct */
        xorl    %edx, %edx
        movl    $STRUCT_SIZE, %edx
        pushl   %edx                    /*Parameters for mp1_copy_from_user from right to left , n*/
        pushl   %ecx                    /*From*/
        pushl   %ebx                    /*To*/
        call    mp1_copy_from_user
        addl    $12, %esp               /*Moving top of stack pointer in correspondance with push*/
        cmpl    $0, %eax                /*Error checks # of bytes mp1_copy_from_user couldn't copy (should be 0)*/
        jne     add_error                      

        /*Checking Location for Errors*/
        xorl    %edx, %edx
        movw    LOCATION(%ebx), %dx     
        cmpw    $0, %dx                 /*Compares %dx with min location offset*/
        jl      add_error
        cmpw    $1999, %dx              /*Compares %dx with max location offset*/
        jg      add_error

        /*Setting Countdown & Status */
        xorl    %eax, %eax              
        movw    ON_LENGTH(%ebx), %ax    
        movw    %ax, COUNTDOWN(%ebx)    /*Setting countdown to ON_LENGTH */
        movw    $1, STATUS(%ebx)        /*Changing STATUS to high */

        /*Inserting struct at head of linked list */
        movl    mp1_list_head, %ecx     /*Moves pointer to current head into %ecx */
        movl    %ecx, NEXT(%ebx)        /*Moves the current head to the struct's next pointer */
        movl    %ebx, mp1_list_head     /*Contents of %ebx (address to struct) transferred to head of list */

        /*Call to mp1_poke */
        xorl    %edx, %edx 
        xorl    %eax, %eax              
        movw    LOCATION(%ebx), %ax     /*Moving value stored in location to %ax */
        movb    ON_CHAR(%ebx), %cl 
        shll    $1, %eax
        call    mp1_poke
        xorl    %eax, %eax              /* mp1_ioctl_add returns 0 if successful add was performed */
        jmp     add_teardown

add_error:
        /* return -1*/
        xorl    %eax, %eax
        dec     %eax                    /* %eax = -1 */
        cmpl    $0, %ebx                /*Compares %ebx with 0 to see if memory needs to be freed */
        jne     add_freemem
        jmp     add_teardown

add_freemem:
        /*Frees malloc'd memory if an error occured*/
        pushl   %ebx                    /*Pushes pointer to malloc'd memory*/
        call    mp1_free
        addl    $4, %esp
        jmp     add_teardown

add_teardown:
        /*Stack teardown*/
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
        ret


/*int mp1_ioctl_remove(unsigned long arg)

 Interface: C-style calling convention (arguments retrieved from stack)
    Inputs: arg(integer location)
   Outputs: returns 0 if success and -1 if error
 Registers: Clobbers %eax, %ecx, %edx
*/
mp1_ioctl_remove:
        /*Creating Stack Frame*/
	pushl   %ebp
        movl    %esp, %ebp 
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        /*Initializing Registers*/
        xorl    %eax, %eax
        xorl    %ebx, %ebx
        xorl    %ecx, %ecx
        xorl    %edx, %edx

        /*Extracting location from argument*/
        movl    8(%ebp), %edx           /*arg(long) -> %edx */
        cmpw    $0, %dx                 /*Checking if location is valid */
        jl      remove_error
        cmpw    $1999, %dx 
        jg      remove_error

        /*Call linked list traversal helper function */
        movl    $1, %ebx                /*Remove bit set high*/
        pushl   %ebx                    /*Pushes parameters to stack */
        pushl   %edx                    /*Holds the location to be found */
        call    mp1_traversal_init
        popl    %edx
        popl    %ebx
        cmpl    $1, %eax
        jne     remove_error            /*If the pointer was null we didn't find a matching location */
        xorl    %eax, %eax              /*Return value = 0*/
        jmp     remove_teardown

remove_error:
        movl   $-1, %eax                /*If error, return = -1 */
remove_teardown:
	/*stack teardown*/
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
        ret

/*
int mp1_ioctl_find(unsigned long arg)

 Interface: C-style calling convention (arguments retrieved from stack)
    Inputs: arg (pointer to struct)
   Outputs: returns -1 if error and 0 if success
 Registers: Clobbers %eax, %ecx, %edx
*/
mp1_ioctl_find:
        /*Creating Stack Frame*/
	pushl   %ebp
        movl    %esp, %ebp 
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        /* Initializing registers */
        xorl    %eax, %eax
        xorl    %ebx, %ebx
        xorl    %ecx, %ecx
        xorl    %edx, %edx

        /*Malloc*/
        pushl   $STRUCT_SIZE            
        call    mp1_malloc              /*Top address returned to  %eax */
        addl    $4, %esp                /*Moving top of stack pointer in correspondance with push*/
        movl    %eax, %ebx              /*Move pointer to %ebx */
        cmpl    $0, %ebx                /*Error check, see if malloc returned null */
        je      find_malloc_error       /*If %ebx is pointing to null, jump to find_malloc_error*/

        /*obtaining argument pointer*/
        movl    8(%ebp), %ecx           /*arg(pointer) -> %ecx */
        cmpl    $0, %ecx 
        je      find_argument_error

        /*copying struct */
        xorl    %edx, %edx
        movl    $STRUCT_SIZE, %edx
        pushl   %edx                    /* parameters for mp1_copy_from_user from right to left , n*/
        pushl   %ecx                    /*from*/
        pushl   %ebx                    /*to*/
        call    mp1_copy_from_user
        addl    $12, %esp               /*moving top of stack pointer in correspondance with push*/
        cmpl    $0, %eax                /*compares number of bytes mp1_copy_from_user was not able to copy (should be zero) */
        jne     find_memcpy_error

        /*Extracting location perameter*/
        movw    LOCATION(%ebx), %dx
        cmpw    $0, %dx                 /*compares location with min value of valid address range */
        jl      find_location_range_error
        cmpw    $1999, %dx              /*compares location with max value of valid address range */
        jg      find_location_range_error

        /*setting up linked list search*/
        movl    $0, %esi                /*remove bit low */
        pushl   %ebx
        pushl   %ecx
        pushl   %esi
        pushl   %edx                    /*location is stored in this register */
        call    mp1_traversal_init
        popl    %edx
        popl    %esi
        popl    %ecx
        popl    %ebx

        cmpl    $0, %eax                /*pointer to matching location element */
        je      find_no_matching_location /*if the pointer was null we didn't find a matching location */

        /*copy to user */ 
        movl    8(%ebp), %ecx           /*arg(pointer) -> %ecx */
        xorl    %edx, %edx
        movl    $STRUCT_SIZE, %edx
        pushl   %edx                    /*parameters for mp1_copy_to_user from right to left , n*/
        pushl   %eax                    /*from*/
        pushl   %ecx                    /*to*/
        call    mp1_copy_to_user
        addl    $12, %esp               /*moving top of stack pointer in correspondance with push*/
        cmpl    $0, %eax                /*compares number of bytes mp1_copy_to_user was not able to copy (should be zero) */
        jne     find_memcpy_error
        jmp     find_teardown

find_no_matching_location:
        movl $-1, %eax
find_malloc_error:
find_memcpy_error:
find_argument_error:
find_location_range_error:
find_teardown:
        /*free original struct memory*/
        pushl   %ebx
        call    mp1_free
        addl    $4, %esp
        
        /*stack teardown*/
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
        ret

/* int mp1_ioctl_sync(unsigned long arg)

 Interface: C-style calling convention (arguments retrieved from stack)
    Inputs: Two 16- bit integer locations stored in passed in through arg
   Outputs: Returns 0 on success and -1 on failure
 Registers: Clobbers %eax, %ecx, %edx
*/
mp1_ioctl_sync:
	
        /*Creating Stack Frame*/
	pushl   %ebp
        movl    %esp, %ebp 
        pushl   %ebx
        pushl   %esi
        pushl   %edi

        /* Initializing registers */
        xorl    %eax, %eax
        xorl    %ebx, %ebx
        xorl    %ecx, %ecx
        xorl    %edx, %edx

        /*extracting locations: %cx: 1st location, %dx: 2nd location */
        movl    8(%ebp), %ecx                   /* arg (long) -> %ecx */
        cmpl    $0, %ecx
        je      sync_location_error
        movw    %cx, %dx                        /* %dx stores the 2nd location */
        shrl    $16, %ecx                       /*right shifts %ecx by 16 bits so the 1st location is now stored completely in %cx */

        /*checking locations*/
        cmpw    $0, %dx                         /*compares %ebx with min value of valid address range */
        jl      sync_location_error
        cmpw    $1999, %dx                      /*compares %ebx with max value of valid address range */
        jg      sync_location_error        
        cmpw    $0, %cx                         /*compares %ebx with min value of valid address range */
        jl      sync_location_error
        cmpw    $1999, %cx                      /*compares %ebx with max value of valid address range */
        jg      sync_location_error      

        /*find element with 1st location */
        movl    $0, %ebx                        /*set remove bit to 0 */
        pushl   %edx                            /*pushing 2nd location to save it in stack */
        pushl   %ebx                            /*pushing remove bit */
        pushl   %ecx                            /*pushing 1st location */
        call    mp1_traversal_init
        popl    %ecx
        popl    %ebx
        popl    %edx
        cmpl    $0, %eax                        /*checks if the returned element is null */
        je      sync_location_error
        movl    %eax, %ebx                      /*pointer to 1st element */

        /*find element with 2nd location */
        xorl    %eax, %eax                      /*initialize eax just in case */
        movl    $0, %ecx                        /*set remove bit to 0 */
        pushl   %ecx                            /*pushing remove bit */
        pushl   %edx                            /*pushing 2nd location */
        call    mp1_traversal_init
        popl    %edx
        popl    %ecx
        cmpl    $0, %eax                        /*checks if returned element is null */
        je      sync_location_error
        movl    %eax, %edx                      /*pointer to the 2nd element */

        /*copy timing information (the on length, off length, countdown, and status fields)*/
        xorl    %eax, %eax
        movw    ON_LENGTH(%ebx), %ax            /* copy on_length */
        movw    %ax, ON_LENGTH(%edx)
        movw    OFF_LENGTH(%ebx), %ax
        movw    %ax, OFF_LENGTH(%edx)
        movw    COUNTDOWN(%ebx), %ax
        movw    %ax, COUNTDOWN(%edx)
        movw    STATUS(%ebx), %ax
        movw    %ax, STATUS(%edx)
        movl    %edx, %ebx                      /*moving pointer to the 2nd element into ebx since edx gets clobbered by mp1_poke*/

        /*assigning character based on status */
        xorl    %eax, %eax
        movw    STATUS(%ebx), %ax
        cmpw    $0, %ax 
        jl      sync_location_error
        je      sync_off_char
        cmpw    $1, %ax
        jg      sync_location_error
        je      sync_on_char

sync_on_char:
        xorb    %cl, %cl
        movb    ON_CHAR(%ebx), %cl
        jmp     sync_poke_call
sync_off_char:
        xorb    %cl, %cl
        movb    OFF_CHAR(%ebx), %cl 
        jmp     sync_poke_call
        /*call mp1_poke */

sync_poke_call:
        xorl    %edx, %edx 
        xorl    %eax, %eax                      /*clearing %eax before use by mp1_poke */
        movw    LOCATION(%ebx), %ax             /* moving value stored in location to %eax */ 
        shll    $1, %eax
        call    mp1_poke
        xorl    %eax, %eax      
        jmp     sync_teardown

sync_location_error:
        movl    $-1, %eax
sync_teardown:
        /*stack teardown*/
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
        ret

/*int mp1_traversal_init(unsigned long location, unsigned long removebit)
 Interface: C-style calling convention (arguments retrieved from stack)
    Inputs: Remove bit from stack, location from stack
   Outputs: If called from remove function, returns 0 for success and 1 for error. 
   If called from find/sync, returns pointer to struct for success and x0000 for error.
   Registers: Clobbers %edx, %ecx, %eax, %edx
*/
mp1_traversal_init:
        /*Creating Stack Frame*/
	pushl   %ebp
        movl    %esp, %ebp 
        pushl   %ebx
        pushl   %esi
        pushl   %edi
        
        movl    $0, %ebx                        /*holds pointer to previous element traversed */
        movl    8(%ebp), %edx                   /*extracting location */
        movl    12(%ebp), %ecx                  /*remove bit. If remove bit is high the element with the matching location is removed from the linked list.*/
        movl    mp1_list_head, %esi             /* pointer to element is at %esi */

traversal_nullchk:
        cmpl    $0, %esi                        /*check if %esi is null */
        jne     traversal_location_check  

traversal_complete:
        /*stack teardown*/
        movl    %esi, %eax
        popl    %edi
        popl    %esi
        popl    %ebx
        leave
        ret

traversal_location_check:
        xorl    %eax, %eax                      /*initializing %eax to 0 in order to use %ax */
        movw    LOCATION(%esi), %ax             /*moves the current element's location value into %ax */
        subw    %dx, %ax                        /* subtracts dx and ax, result is stored in ax. */
        cmpw    $0, %ax                         /*If dx and ax are equal, then ax should be 0. */
        je      traversal_remove                /*if dx and ax are equal, we found the matching element. Now we check if we need to remove the element or not  */

traversal_next:
        movl    %esi, %ebx                      /*moves pointer to previous head into %ebx */
        movl    NEXT(%esi), %esi
        jmp     traversal_nullchk

traversal_remove:
        cmpl    $1, %ecx
        jne     traversal_complete 
        cmpl    $0, %ebx                        /* sees if the element to be removed was the first element in the list */
        je      traversal_rewrite_header
        movl    NEXT(%esi), %eax                /*moves pointer to the next element to %eax */
        movl    %eax, NEXT(%ebx)                /*moves pointer to the next element to the next pointer of the previous element */
        jmp     traversal_freemem

traversal_rewrite_header:
        movl    NEXT(%esi), %eax
        movl    %eax, mp1_list_head
        jmp     traversal_freemem

traversal_freemem:
        pushl   %esi
        call    mp1_free
        addl    $4, %esp
        movl    $1, %esi
        jmp     traversal_complete
.end
