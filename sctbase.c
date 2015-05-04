#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdbool.h>
#include "CRC_simple.c"
#define MAX_SIZE 78

typedef struct Timer{
    bool start;
    int count;
    int limit;
}Timer;
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

int compute_crc_with_newline(const char* message){
    int crc;
    char temp[78];
    strcpy(temp,message);
    strcat(temp,"\n");
    crc = (int)CRCpolynomial(temp);
    return crc;
}

void save_line(char* buffer , FILE *fout){
    char temp[78];
    char tok_buffer[78];
    char rest_buffer[78];
    scttok(buffer,tok_buffer,rest_buffer);
    scttok(rest_buffer,tok_buffer,rest_buffer);
    strcpy(temp,rest_buffer);
    printf("DATA: %s\n",temp);
    if(fout!=NULL) {
        fprintf(fout,"%s\n",temp);
        printf("syst: line saved:%s\n",temp);
    }
    else{
        printf("error during writing...\n");
        exit(0);
    }
}

int get_ack_op_stats(const char* message , char* stats){
    char rest[78];
    scttok(message,stats,rest);
    int ack = 0;
    ack = atoi(rest);
    return ack;
}
