#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#include "mylib.c"

#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;


#include "UDP_supporting_functions_2015.c"
#define BUFFESIZE 80
//rememver, the BUFFESIZE has to be at least big enough to receive the answer from the server
#define SEGMENTSIZE 78
#define TIMERLIMIT 4
#define WINDOWSIZE 4
struct sockaddr_in localaddr,remoteaddr;
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.


// Simple usage: client IP port, or client IP (use default port)
int main(int argc, char *argv[]) {
//********************************************************************
// WSSTARTUP
//********************************************************************
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}
//*******************************************************************
// Initialization
//*******************************************************************
    memset(&localaddr, 0, sizeof(localaddr));//clean up
    //char localIP[INET_ADDRSTRLEN]="127.0.0.1";
    int localPort=1234;
    localaddr.sin_family = AF_INET;
    //localaddr.sin_addr.s_addr = inet_addr(localIP);
    localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
//********************************************************************
    localaddr.sin_port = htons(localPort);
    memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up
    randominit();

    SOCKET s;

    char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
    remoteaddr.sin_family = AF_INET;
//*******************************************************************
//	Dealing with user's arguments
//*******************************************************************
    if (argc != 5) {
        printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
        exit(1);
    }
    remoteaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP address
    remoteaddr.sin_port = htons((u_short)atoi(argv[2]));//always get the port number
    //localaddr.sin_port = htons((u_short)atoi(argv[3]));//always get the port number
    packets_lostbit=atoi(argv[3]);
    packets_damagedbit=atoi(argv[4]);
    if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1){
        printf("2011 code USAGE: client remote_IP-address port  lossesbit(0 or 1) damagebit(0 or 1)\n");
        exit(0);
    }

//*******************************************************************
//CREATE CLIENT'S SOCKET
//*******************************************************************
    s = socket(AF_INET, SOCK_DGRAM, 0);//this is a UDP socket
    if (s < 0) {
        printf("socket failed\n");
        exit(1);
    }

    //***************************************************************//
    //NONBLOCKING OPTION for Windows
    //***************************************************************//
    u_long iMode=1;
    ioctlsocket(s,FIONBIO,&iMode);

//*******************************************************************
//SEND A TEXT FILE
//*******************************************************************
    //int counter=0;//sequence of packets
    int ackcounter=0;
    char temp_buffer[BUFFESIZE];

    FILE *fin=fopen("file1.txt","r");

    if(fin==NULL){
        printf("cannot open file\n");
        exit(0);
    }

    Timer t;
    t.start = true;
    t.count = 0;
    t.limit = TIMERLIMIT;

    int base = 0;
    int nextseqnum = 0;

    //--buffer file into an string array, packet-ize it
    char sndpkt[100][SEGMENTSIZE];
    int memsetcount = 0;
    for(;memsetcount<100;memsetcount++){
        memset(sndpkt[memsetcount],0,sizeof(sndpkt[memsetcount]));
    }
    buffer_file_to_array(sndpkt,fin);
    array_pktize(sndpkt);
    //--file buffer done, all pkt prepared

    char tok_buffer[78];
    char rest_buffer[78];

    int ack = 0;


    while (1){
        memset(tok_buffer,0,sizeof(tok_buffer));
        memset(rest_buffer,0,sizeof(rest_buffer));
        //timer runs if enabled
        if(t.start == true){
            t.count++;
            printf("Timer: %d\n",t.count);
        }

        if(nextseqnum<base+WINDOWSIZE){
            send_unreliably(s,sndpkt[nextseqnum],remoteaddr);
            if(base == nextseqnum){
                t.start = true;
            }
            nextseqnum++;
        }else{
            Sleep(1000);
        }

//FSM 2
        if(t.count >= t.limit){
            t.count = 0;
            t.start = true;

            int timeoutcount = base;
            for(;timeoutcount < nextseqnum;timeoutcount++){
                send_unreliably(s,sndpkt[timeoutcount],remoteaddr);
            }
            printf("syst: timeout resend finished\n");
            Sleep(1000);
        }

//FSM 3
        Sleep(100);
        //receive the pkt
        memset(receive_buffer, 0, sizeof(receive_buffer));
        recv_nonblocking(s,receive_buffer, remoteaddr);
        printf("RECEIVE --> %s \n",receive_buffer);

        //see if pkt corrupted
        int recv_crc , calc_crc;
        mytok(receive_buffer,tok_buffer,rest_buffer);
        calc_crc = (int)CRCpolynomial(rest_buffer);
        printf("computed crc is %d\n",calc_crc);
        recv_crc = atoi(tok_buffer);
        printf("received crc is %d\n",recv_crc);

        //if pkt good
        if(calc_crc == recv_crc){
            printf("crc match\n");
            //if received a ACk not a CLS, base = ack + 1
            if(strncmp(rest_buffer,"ACK",3)==0){
                mytok(rest_buffer,tok_buffer,rest_buffer);
                ack = atoi(rest_buffer);

                base = ack+1;
                //if base reaches nextseqnum, stop timer
                if(base == nextseqnum)
                    t.start = false;
                else{
                    //t.count = 0;
                    t.start = true;
                }
            //if received a CLS
            }else if(strncmp(rest_buffer,"CLS",3)==0){
                printf("received CLS ack from receiver, out.\n");
                break;
            }else{
                printf("receiving wrong\n");
                //while(1);
            }
            //...
        }else{
            printf("crc not match\n");
            //ACK corrupted
        }


/*
        memset(send_buffer, 0, sizeof(send_buffer));//clean up the send_buffer before reading the next line
        fgets(send_buffer,SEGMENTSIZE,fin);
        if (!feof(fin)) {
            //add a header to the packet with the sequence number
            //sprintf(temp_buffer,"PACKET %d ",counter);
            memset(sndpkt[nextseqnum] , 0 , sizeof(sndpkt[nextseqnum]));
            make_pkt(nextseqnum,send_buffer,sndpkt[nextseqnum]);
            //strcat(temp_buffer,send_buffer);
            //strcpy(send_buffer,temp_buffer);
            send_unreliably(s,sndpkt[nextseqnum], remoteaddr);
            nextseqnum++;

            Sleep(200);

            //********************************************************************
            //RECEIVE
            //********************************************************************
            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);//you can replace this, but use MSG_DONTWAIT to get non-blocking recv
            printf("RECEIVE --> %s \n",receive_buffer);

            //--deal with receive ack pkt

            int recv_crc , calc_crc;
            mytok(receive_buffer,tok_buffer,rest_buffer);
            calc_crc = (int)CRCpolynomial(rest_buffer);
            printf("computed crc is %d\n",calc_crc);
            recv_crc = atoi(tok_buffer);
            printf("received crc is %d\n",recv_crc);
            if(calc_crc == recv_crc){
                printf("crc match\n");
                //...
            }else{
                printf("crc not match\n");
                //ACK corrupted
            }




            Sleep(200);

        }
        else {
            printf("end of the file \n");
            memset(send_buffer, 0, sizeof(send_buffer));
            sprintf(send_buffer,"CLOSE \r\n");
            send_unreliably(s,send_buffer,remoteaddr);//we actually send this reliably, read UDP_supporting_functions_2012.c

            break;
        }
    }
*/
//*******************************************************************
//CLOSESOCKET
//*******************************************************************
    }
    printf("closing everything on the client's side ... \n");
    fclose(fin);

    closesocket(s);


    exit(0);

}

