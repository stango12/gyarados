#include "syscall.h"

file_array files_array[MAX_FILES];

uint32_t* stdin_ops_table[4] = {NULL, (uint32_t *) terminal_read, NULL, NULL};
uint32_t* stdout_ops_table[4] = {NULL, NULL, (uint32_t *) terminal_write, NULL};
uint32_t* rtc_ops_table[4] = {(uint32_t *) rtc_open, (uint32_t *) rtc_read, (uint32_t *) rtc_write, (uint32_t *) rtc_close};
uint32_t* dir_ops_table[4] = {(uint32_t *) dir_open, (uint32_t *) dir_read, (uint32_t *) dir_write, (uint32_t *) dir_close};
uint32_t* files_ops_table[4] = {(uint32_t *) fs_open, (uint32_t *) fs_read, (uint32_t *) fs_write, (uint32_t *) fs_close};


uint32_t files_in_use = 2;

int32_t halt (uint8_t status) {
    return -1;
}

int32_t execute (const uint8_t * command) {
    int i = 0;
    int j = 0;

    // declare the data associated with the filename
    uint8_t file_name[MAX_FILENAME_LENGTH];
    char data[32][32];
    int curr_data_index = 0;
    int curr_str_index = 0;

    // declare magic numbers 
    uint8_t buf[NUM_BYTES_STATS];
    uint8_t magic_nums[NUM_MAGIC_NUMS] = {MAGIC_NUM_1, MAGIC_NUM_2, MAGIC_NUM_3, MAGIC_NUM_4};

    uint32_t entry_point_addr = 0;
    uint32_t curr_read_entry_point = ENTRY_POINT_START;

    // error if command string is null
    if(command == NULL || command == NULL_CHAR) {
        return -1;
    }

    for(i = 0; i < 32; i++) {
        for(j = 0; j < 32; j++) {
            data[i][j] = ' ';
        }
    }

    for(i = 0; i < strlen(command); i++) {
        if(command[i] == ' ') {
            data[curr_data_index][curr_str_index] = '\0';
            curr_data_index++;
            curr_str_index = 0;
        } else {
            data[curr_data_index][curr_str_index] = command[i];
            curr_str_index++;
        }
    }

    // copy over filename including null termination
    strcpy((int8_t *) file_name, (int8_t *) data[0]);

    // read the first four bytes of the file
    if(-1 == fs_read((int8_t*) file_name, buf, NUM_BYTES_STATS)) {
        return -1;
    }

    // check to see if its executable
    if(0 != strncmp((int8_t *) buf, (int8_t *) magic_nums, NUM_MAGIC_NUMS)) {
        return -1;
    }

    // get entry point into program
    entry_point_addr |= (buf[curr_read_entry_point] << (3 * 8));
    curr_read_entry_point++;
    entry_point_addr |= (buf[curr_read_entry_point] << (2 * 8));
    curr_read_entry_point++;
    entry_point_addr |= (buf[curr_read_entry_point] << (1 * 8));
    curr_read_entry_point++;
    entry_point_addr |= (buf[curr_read_entry_point]);
    curr_read_entry_point++;

    // set up the new page directory
    
    // load the program into the correct starting address
    copy_file_to_addr(data[0], PROGRAM_EXEC_ADDR);
    
    // assign a pcb with the new PID
    
    // save things
    
    // jump to the file to execute it
    user_exec(entry_point_addr);

    return 0;
}

int32_t read (int32_t fd, void * buf, int32_t nbytes) {
    return -1;
}

int32_t write (int32_t fd, const void * buf, int32_t nbytes) {
    return -1;
}

int32_t open (const uint8_t * filename) {
    if(strncmp("stdin", filename, 5) == 0){
        files_array[0].operations_pointer = stdin_ops_table;
        files_array[0].inode = NULL;
        files_array[0].flags = IN_USE;
    }

    if(strncmp("stdout", filename, 6) == 0){
        files_array[1].operations_pointer = stdout_ops_table;
        files_array[1].inode = NULL;
        files_array[1].flags = IN_USE;
    }

    dentry_t file_info;
    int32_t check = read_dentry_by_name(filename, &file_info);

    //checking if the file name exists
    if(check == -1 || files_in_use > MAX_FILES) {
        return -1;
    }

    int i = 0;
    for(i = 2; i < MAX_FILES; i++) {
        if(files_array[i].flags == NOT_USE) {
            switch(file_info.file_type) {
                case 0:
                    files_array[i].operations_pointer = rtc_ops_table;
                    files_array[i].inode = NULL;
                    files_array[i].file_position = 0;
                    break;
                case 1:
                    files_array[i].operations_pointer = dir_ops_table;
                    files_array[i].inode = NULL;
                    files_array[i].file_position = 0;
                    break;
                case 2:
                    files_array[i].operations_pointer = files_ops_table;
                    files_array[i].inode = file_info.inode_num;
                    files_array[i].file_position = 0;
                    break;
            }

            files_array[i].flags = IN_USE;
            files_in_use++;
            return i;
        }
    }    


    return -1;
}

int32_t close (int32_t fd) {
    if(fd >= 2 && fd <= 7)
    {
        files_array[fd].flags = NOT_USE;
        files_in_use--;
        return 0;
    }
    return -1;
}

int32_t getargs (uint8_t * buf, int32_t nbytes) {
    return -1;
}

int32_t vidmap (uint8_t ** screen_start) {
    return -1;
}

int32_t set_handler (int32_t signum, void * handler_address) {
    return -1;
}

int32_t sigreturn (void) {
    return -1;
}