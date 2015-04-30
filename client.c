#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <stdbool.h>
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;
#include "UDP_supporting_functions_2015.c"
#include "CRC_simple.c"
#define BUFFERSIZE 80
#define SEGMENTSIZE 78
#define WINDOWSIZE 4
#define TIMERLIMIT 10
struct sockaddr_in localaddr,remoteaddr;

void make_pkt(char* send_buffer, int counter);
bool check_corrupt(char* rcv_buffer);
char* getackstats(char* receive_buffer);

int main(int argc, char *argv[]) {
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}

    //--initiate
    memset(&localaddr, 0, sizeof(localaddr));//clean up
    int localPort=1234;
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = INADDR_ANY;
    localaddr.sin_port = htons(localPort);
    memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up
    randominit();

    SOCKET s;

    char send_buffer[BUFFERSIZE],receive_buffer[BUFFERSIZE];
    remoteaddr.sin_family = AF_INET;

    if (argc != 5) {
        printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
        exit(1);
    }
    remoteaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP address
    remoteaddr.sin_port = htons((u_short)atoi(argv[2]));//always get the port number
    packets_lostbit=atoi(argv[3]);
    packets_damagedbit=atoi(argv[4]);
    if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1){
	   printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
	   exit(0);
    }

    //--create client socket
    s = socket(AF_INET, SOCK_DGRAM, 0);//this is a UDP socket
    if (s < 0) {
        printf("socket failed\n");
        exit(1);
    }

    u_long iMode=1;
    ioctlsocket(s,FIONBIO,&iMode);

    //-send text file
    int counter=0;//sequence of packets
    int ackcounter=0;
    char temp_buffer[BUFFERSIZE];

    FILE *fin=fopen("file1.txt","r");
    if(fin==NULL){
        printf("cannot open file\n");
        exit(0);
    }
////////////////////////////////////all the stuff above no need touch////////////////////////////////////////////////////////////
    //initiation for GBN
    int base = 0;
    int nextseqnum = 0;
    int checksum = 0;
    char sndpkt[80][BUFFERSIZE];

    //--define a Timer structure with 2 members, a start status and a count number
    //--and declare a Timer type variable t.
    typedef struct Timer{
        bool start;
        int count;
        int limit;
    }Timer;
    Timer t;
    t.start = false;
    t.count = 0;
    t.limit = TIMERLIMIT;

    while (1){
        //--the fake timer runs if its enabled, not a part of the FSM
        if(t.start == true)
            t.count++;

        //-- if timeout
        if(t.count >= t.limit){
            t.count = 0;
            t.start = true;

            int i = base;
            for(;i<nextseqnum-1;i++)
                send_unreliably(s,sndpkt[i],remoteaddr);
        }

        memset(send_buffer, 0, sizeof(send_buffer));

        if(!feof(fin)){
            if(nextseqnum<base+WINDOWSIZE){
                fgets(send_buffer,SEGMENTSIZE,fin);
                if(!feof(fin)){
                    make_pkt(send_buffer,nextseqnum);
                    //sndpkt[nextseqnum] = make_pkt(send_buffer,nextseqnum);
                    strcpy(sndpkt[nextseqnum],send_buffer);
                    send_unreliably(s,sndpkt[nextseqnum],remoteaddr);
                    if(base == nextseqnum)
                        t.start = true;
                    nextseqnum++;
                }
            }
        }else{
            printf("end of the file \n");
            sprintf(send_buffer,"CLOSE %d\r\n",nextseqnum);
            send_unreliably(s,send_buffer,remoteaddr);//we actually send this reliably, read UDP_supporting_functions_2012.c
        }

        recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
        printf("RECEIVE --> %s \n",receive_buffer);
        if(check_corrupt(receive_buffer)){
            if(strcmp(getackstats(receive_buffer),"CLS")==0)
                break;
            base = getacknum(receive_buffer)+1;
            if(base == nextseqnum)
                t.start = false;
            else
                t.start = true;
        }


        /*
        memset(send_buffer, 0, sizeof(send_buffer));//clean up the send_buffer before reading the next line
        fgets(send_buffer,SEGMENTSIZE,fin);
        if (!feof(fin)) {
            //add a header to the packet with the sequence number
            make_pkt(send_buffer,counter);
            counter++;
            send_unreliably(s,send_buffer, remoteaddr);
            Sleep(1000);

            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
            printf("RECEIVE --> %s \n",receive_buffer);
            Sleep(1000);
        }
        else {
            printf("end of the file \n");
            memset(send_buffer, 0, sizeof(send_buffer));
            sprintf(send_buffer,"CLOSE \r\n");
            send_unreliably(s,send_buffer,remoteaddr);//we actually send this reliably, read UDP_supporting_functions_2012.c

            break;
        }
        */
    }
    //--close socket
    printf("closing everything on the client's side ... \n");
    fclose(fin);
    closesocket(s);
    exit(0);
}

bool check_corrupt(char* rcv_buffer){
    char* temp;
    temp = strtok(rcv_buffer," ");
    if((strcmp(temp,"ACK")==0)||(strcmp(temp,"CLS")==0))
        return true;
    else
        return false;
}

void make_pkt(char* send_buffer, int counter){
    char temp_buffer[80];
    //char temp_buffer2[80];
    sprintf(temp_buffer,"PACKET %d ",counter);
    strcat(temp_buffer,send_buffer);

    unsigned int crc_value;
    crc_value = CRCpolynomial(temp_buffer);
    sprintf(send_buffer,"%u ",crc_value);
    strcat(send_buffer,temp_buffer);

   // return temp_buffer2;
}

char* getackstats(char* receive_buffer){
    char* temp;
    temp = strtok(receive_buffer," ");
    return temp;
}

int getacknum(char* receive_buffer){
    char* temp;
    temp = strtok(receive_buffer," ");
    temp = strtok(NULL," ");
    int acknum;
    acknum = atoi(receive_buffer);
    return acknum;
}

