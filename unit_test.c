#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sctbase.c"
#define BUFFESIZE 80
#define SEGMENTSIZE 78
#define NUMBER_OF_WORDS_IN_THE_HEADER 2

int main(){
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
