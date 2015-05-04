#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "CRC_simple.c"
#include <time.h>
#include <stdbool.h>

//void print_bin(int input);
//int make_twobyte(char high, char low);
char* testtok(char* recv_buffer);
bool check_corrupt(char* rcv_buffer);
char* make_pkt(char* send_buffer, int counter);
int getcrc(char* recv_buffer);
char* substring(char* source, int start);
char* getrest(char* recv_buffer);


int main(){
    char receive_buffer[80] = "12345 shangshandalaohu";
    int crc;
    crc = getcrc(receive_buffer);
    char* rest;
    printf("receive_buffer is %s\n",receive_buffer);
    rest = getrest(receive_buffer);
    printf("crc is %d\n",crc);
    printf("rest is %s\n",rest);
}


char* getrest(char* recv_buffer){
    char temp[80];
    strcpy(temp,recv_buffer);

    char* tok = strtok(temp," ");
    int len;
    len = strlen(tok)+1;

    char* result;
    result = substring(recv_buffer,len);
    printf("result is %s \n",result);
    return result;
}

char* substring(char* source, int start){
    char temp[80];
    int n;
    int len;
    len = strlen(source) - start;
    //printf("len = %d\n",len);
    for(n = 0; n < len; n++){
        temp[n] = source[start+n];
    }
    temp[n] = '\0';
    printf("substring output is: %s\n",temp);
    return temp;
}



char* make_pkt(char* send_buffer, int counter){
    char temp_buffer[80];
    char temp_buffer2[80];
    sprintf(temp_buffer,"PACKET %d ",counter);
    strcat(temp_buffer,send_buffer);

    unsigned int crc_value;
    crc_value = CRCpolynomial(temp_buffer);
    sprintf(temp_buffer2,"%u ",crc_value);
    strcat(temp_buffer2,temp_buffer);

    return temp_buffer2;
}

int getcrc(char* recv_buffer){
    char copy[80];
    strcpy(copy,recv_buffer);
    char* temp;
    temp = strtok(copy, " ");
    int crc;
    crc = atoi(temp);
    return crc;
}

char* testtok(char* recv_buffer){
    char* temp;
    temp = strtok(recv_buffer," ");
    return temp;
}
/*
//display 16bit binary from an int
void print_bin(int input){
    char bin[17];
    int a;
    int n;
    for(n = 0; n <= 15 ; n++){
        a = pow(2,(15-n));
        if(input >= a){
            input -= a;
            bin[n] = '1';
        }else
            bin[n] = '0';
    }
    bin[16] = '\0';
    printf("binary: %s \n", bin);
}
*/
bool check_corrupt(char* rcv_buffer){
    char* temp;
    temp = strtok(rcv_buffer," ");
    if((strcmp(temp,"ACK")==0)||(strcmp(temp,"CLS")==0))
        return true;
    else
        return false;
}
