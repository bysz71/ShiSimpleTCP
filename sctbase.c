#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "CRC_simple.c"
#define MAX_SIZE 78

void print_char_array(char array[][MAX_SIZE]);



void buffer_file_to_array(char strarr[][MAX_SIZE] , FILE *fin){
    char temp[70] = "";
    int n =0;
    while(1){
        memset(temp,0,sizeof(temp));
        fgets(temp,78,fin);
        if(!feof(fin)){
            strcpy(strarr[n],temp);
            n++;
        }else{
            break;
        }
    }
    printf("buffer done.\n");
    //print_char_array(two_d_array);
}

//--print an array of string, include all elements with or without content
//--elements without content, the length would be 0
void print_char_array(char array[][MAX_SIZE]){
    int n = 0;
    for(;n<100;n++){
        printf("char_array[%d]:%s\n",n,array[n]);
    }
}

void make_pkt(int nextseqnum , const char* header , const char* data ,char* pkt ){
    char temp[78]="";
    sprintf(temp , header , nextseqnum);
    strcat(temp,data);
    int crc;
    crc = (int)CRCpolynomial(temp);
    sprintf(pkt, "%d ", crc);
    strcat(pkt,temp);
}

void char_array_pktize(char array[][MAX_SIZE] , char* header){
    int n = 0;
    for(;n<100;n++){
        if(strlen(array[n])>0)
            make_pkt(n,header,array[n],array[n]);
        else{
            make_pkt(n,"CLOSE","\n",array[n]);
            break;
        }
    }

}
