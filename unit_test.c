#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sctbase.c"
#define BUFFESIZE 80
#define SEGMENTSIZE 78
#define NUMBER_OF_WORDS_IN_THE_HEADER 2

int main(){

/*
//-get_seqnum_output_header_data--
    char buffer[78] = "PACKET 6 this is the data\n";
    int seqnum = 0;
    char header[78],data[78];
    seqnum = get_seqnum_output_header_data(buffer,header,data);
    printf("seqnum:%d;\nheader:%s;\ndata:%s;\n",seqnum,header,data);
/*
//-get_crc_output_rest test--
    char buffer[78] = "12345 this is a joke\n";
    char message[78] = "";
    int crc_rcv = get_crc_output_rest(buffer,message);
    printf("crc is:%d\n",crc_rcv);
    char buffer2[78] = "abcde this is a joke\n";
    memset(message,0,sizeof(message));
    crc_rcv = get_crc_output_rest(buffer2,message);
    printf("crc is:%d\n",crc_rcv);
    char buffer3[78] = "123de this is a joke\n";
    memset(message,0,sizeof(message));
    crc_rcv = get_crc_output_rest(buffer3,message);
    printf("crc is:%d\n",crc_rcv);
//-scttok test---
/*
    char buffer[78] = "this is a joke\n";
    char tok[78];
    char rest[78];
    scttok(buffer,tok,rest);
    printf("buffer:-%s-\ntok:-%s-\nrest:-%s-\n",buffer,tok,rest);

//--crc test----
/*
    int crc;
    char buffer[7] = "CLOSE\n";
    crc = (int)CRCpolynomial(buffer);
    printf("%d",crc);
//--make_pkt test----------------
/*
    int nextseqnum = 12;
    char sndpkt[78];
    char data[78] = "break the circle of life\n";
    make_pkt(nextseqnum,"PACKET ",data,sndpkt);
    printf("%s",sndpkt);
    */
}
