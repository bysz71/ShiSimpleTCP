#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;

#include "UDP_supporting_functions_2015.c"
#include "sctbase.c"
#define BUFFESIZE 80
#define SEGMENTSIZE 78
struct sockaddr_in localaddr,remoteaddr;

int main(int argc, char *argv[]) {
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}
    memset(&localaddr, 0, sizeof(localaddr));//clean up
    int localPort=1234;
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
    localaddr.sin_port = htons(localPort);
    memset(&remoteaddr, 0, sizeof(remoteaddr));//clean up
    randominit();
    SOCKET s;
    char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
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

    s = socket(AF_INET, SOCK_DGRAM, 0);//this is a UDP socket
    if (s < 0) {
        printf("socket failed\n");
        exit(1);
    }

    //windows nonblocking option
    u_long iMode=1;
    ioctlsocket(s,FIONBIO,&iMode);




//send and receive loop start here+++++++++++++++++++++++++++++++++++++++++++++++
    int counter=0;//sequence of packets
    int ackcounter=0;
    char temp_buffer[BUFFESIZE];

    //--file operation--
    FILE *fin=fopen("file1.txt","r");
    if(fin==NULL){
        printf("cannot open file\n");
        exit(0);
    }
    //--file operation--prepare an char[][]
    char sndpkt[100][SEGMENTSIZE];
    int memsetcount = 0;
    for(;memsetcount<100;memsetcount++){
        memset(sndpkt[memsetcount],0,sizeof(sndpkt[memsetcount]));
    }
    buffer_file_to_array(sndpkt,fin);
    char_array_pktize(sndpkt);
    print_char_array(sndpkt);
    printf("----------------test good------------------\n");
    Sleep(1000);


//my loop -- with ack verification and resend
    int nextseqnum = 0;
    int ack = -1;
    int crc_rcv = 0;
    int crc_cal = 0;
    char message[78] = "";
    char stats[78] = "";

    while(1){
        if(strlen(sndpkt[nextseqnum])>0){
            send_unreliably(s,sndpkt[nextseqnum],remoteaddr);
            //printf("%d pkt--%s-- sent\n",nextseqnum,sndpkt[nextseqnum]);
            Sleep(400);

            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);
            printf("RECEIVE --> %s \n",receive_buffer);

            memset(message,0,sizeof(message));
            crc_rcv = get_crc_op_rest(receive_buffer,message);
            printf("crc_rcv:%d\n",crc_rcv);
            //crc_cal = (int)CRCpolynomial(message);
            crc_cal = compute_crc_with_newline(message);    //receive_nonblocking() contains the function to get rid of \n
            printf("crc_cal:%d\n",crc_cal);
            ack = get_ack_op_stats(message,stats);
            printf("ack:%d\n",ack);
            printf("stats:%s\n",stats);

            if(crc_rcv == crc_cal){
                if(ack == nextseqnum)
                    nextseqnum++;

            }
            Sleep(400);
        }else{
            printf("at %d, end reached\n",nextseqnum);
            break;

        }
    }



//*my loop -- simplest one
/*
    int nextseqnum = 0;
    while(1){
        if(strlen(sndpkt[nextseqnum])>0){
            send_unreliably(s,sndpkt[nextseqnum],remoteaddr);
            //printf("%d pkt--%s-- sent\n",nextseqnum,sndpkt[nextseqnum]);
            nextseqnum++;
            Sleep(400);

            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);
            printf("RECEIVE --> %s \n",receive_buffer);
            Sleep(400);
        }else{
            printf("at %d, end reached\n",nextseqnum);
            break;

        }
    }



/* lecture's loop
    while (1){
        memset(send_buffer, 0, sizeof(send_buffer));
        fgets(send_buffer,SEGMENTSIZE,fin);
        if (!feof(fin)) {
            sprintf(temp_buffer,"PACKET %d ",counter);
            counter++;
            strcat(temp_buffer,send_buffer);
            strcpy(send_buffer,temp_buffer);
            send_unreliably(s,send_buffer, remoteaddr);
            Sleep(1000);

            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);
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
    }
*/
//loop ends+++++++++++++++++++++++++++++++++++++++++++++++++++++



    printf("closing everything on the client's side ... \n");
    fclose(fin);
    closesocket(s);

    exit(0);

}

