#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

void printf_buff(char * info, unsigned char * buff, int buff_size){
    printf("%s buff:", info);
    for(int i = 0; i < buff_size; i++){
        printf("%02X ", buff[i]);
    }
    printf("\n");
}