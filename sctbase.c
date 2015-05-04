#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "CRC_simple.c"
#define MAX_SIZE 78

void print_char_array(char array[][MAX_SIZE]);

void scttok(const char* source, char* tok, char* rest){
    if(strlen(source)==0)
        return;
    char cpy[MAX_SIZE];
    strcpy(cpy,source);

    int n = 0;
    for(;cpy[n]!=' ';n++)
        tok[n] = cpy[n];
    tok[n] = '\0';

    n++;
    int m = 0;
    for(;n < strlen(source) ; n++){
        rest[m] = source[n];
        m++;
    }
    rest[m] = '\0';
}

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
    sprintf(temp , "%s %d " ,header, nextseqnum);
    strcat(temp,data);
    int crc;
    crc = (int)CRCpolynomial(temp);
    sprintf(pkt, "%d ", crc);
    strcat(pkt,temp);
}

void char_array_pktize(char array[][MAX_SIZE]){
    int n = 0;
    for(;n<100;n++){
        if(strlen(array[n])>0)
            make_pkt(n,"PACKET",array[n],array[n]);
        else{
            make_pkt(n,"CLOSE","\n",array[n]);
            break;
        }
    }

}

int get_crc_op_rest(const char* pkt, char* message){
    char tok[MAX_SIZE];
    scttok(pkt,tok,message);
    int crc;
    crc =atoi(tok);
    return crc;
}

int get_seqnum_op_header_data(const char* source,char* header, char* data){
    char tok[MAX_SIZE];
    char rest[MAX_SIZE];
    int seqnum = 0;
    scttok(source,tok,rest);
    strcpy(header,tok);
    scttok(rest,tok,data);
    seqnum = atoi(tok);

    return seqnum;
}

