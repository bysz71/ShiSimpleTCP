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


//*my loop
    int nextseqnum = 0;
    while(1){
        if(strlen(sndpkt[nextseqnum])>0){
            send_unreliably(s,sndpkt[nextseqnum],remoteaddr);
            printf("%d pkt--%s-- sent\n",nextseqnum,sndpkt[nextseqnum]);
            nextseqnum++;
            Sleep(400);

            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);
            printf("RECEIVE --> %s \n",receive_buffer);
            Sleep(400);
        }else{
            printf("at %d, end reached\n");
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

