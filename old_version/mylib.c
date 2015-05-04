#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "CRC_simple.c"
#define MAX_SIZE 78
void mytok(const char* source, char* tok, char* rest);
void chopoffnewline(const char* input, char* output);
void print_chararray(char array[][MAX_SIZE]);

typedef struct Timer{
    bool start;
    int count;
    int limit;
}Timer;


void save_line(char* buffer , FILE *fout){
    char temp[78];
    char tok_buffer[78];
    char rest_buffer[78];
    mytok(buffer,tok_buffer,rest_buffer);
    mytok(rest_buffer,tok_buffer,rest_buffer);
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

void make_pkt_ACK(int expectseqnum , char* pkt){
    char temp[78] = "";
    sprintf(temp , "ACK %d" , expectseqnum);
    int crc;
    crc = (int)CRCpolynomial(temp);
    sprintf(pkt,"%d ",crc);
    strcat(pkt,temp);
}

void make_pkt_CLS(int expectseqnum , char* pkt){
    char temp[78] = "";
    sprintf(temp, "CLS %d", expectseqnum);
    int crc;
    crc = (int)CRCpolynomial(temp);
    sprintf(pkt,"%d ",crc);
    strcat(pkt,temp);
}

void make_pkt(int nextseqnum , const char* data ,char* pkt ){
    char temp[78]="";
    sprintf(temp , "PACKET %d " , nextseqnum);
    strcat(temp,data);
    int crc;
    char temp2[78] = "";
    chopoffnewline(temp,temp2);
    crc = (int)CRCpolynomial(temp2);
    sprintf(pkt, "%d ", crc);
    strcat(pkt,temp);
}

void make_pkt_close(int nextseqnum, char* pkt){
    int crc;
    char temp[78] = "";
    sprintf(temp , "CLOSE %d end of file\n",nextseqnum);
    char temp2[78];
    chopoffnewline(temp,temp2);
    crc = (int)CRCpolynomial(temp2);
    sprintf(pkt,"%d ",crc);
    strcat(pkt,temp);

}

void mytok(const char* source, char* tok, char* rest){
    if(strlen(source)==0)
        return;
    char cpy[80];
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

void chopoffnewline(const char* input, char* output){
    int i = 0;
    while(1){
        if(input[i]=='\n'){
            output[i] = '\0';
            return;
        }
        else
            output[i] = input[i];
        i++;
    }
}

void array_pktize(char strarr[][MAX_SIZE]){
    char temp[78]="";
    int n = 0;
    while(n<100){
        strcpy(temp,strarr[n]);
        if(strlen(temp)>0){
            make_pkt(n,temp,strarr[n]);
            n++;
        }else{
            make_pkt_close(n,strarr[n]);
            break;
        }
    }
    //print_chararray(strarr);

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
    //print_chararray(two_d_array);
}

//--print an array of string, include all elements with or without content
//--elements without content, the length would be 0
void print_chararray(char array[][MAX_SIZE]){
    int n = 0;
    for(;n<100;n++){
        printf("chararray[%d]:%s\n",n,array[n]);
    }
}
