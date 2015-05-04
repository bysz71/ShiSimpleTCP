//12238967 Clark Zhang
//sorry Sir, the definitions for unix and apple systems were
//removed for reading convenience, please compile under win32.
//All other files in the folder are required for compilation.
//under terrible network environment, the smaller window size,
//the better performance. I think it is because with high
//probability of invalid packet, small windows size sends less
//redundant packets which will be discarded anyway
//fake timer used, one loop count is treated as one time unit

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;

#include "UDP_supporting_functions_2015.c"
#include "sctbase.c"
#define BUFFESIZE 80
#define SEGMENTSIZE 78
#define WINDOWSIZE 2
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
    //--buffer whole file to sndpkt[][]
    buffer_file_to_array(sndpkt,fin);
    //--make_pkt for every string in sndpkt[]
    char_array_pktize(sndpkt);
    int pkt_count = 0;
    //--find out number of packets need to send
    while(strlen(sndpkt[pkt_count])>1){
        pkt_count++;
    }


//my loop -- with ack verification and resend
    int base = 0;
    int nextseqnum = 0;
    int ack = -1;
    int ack_old = -1;
    int crc_rcv = 0;
    int crc_cal = 0;
    int timeout_count = 0;
    int loop_count = 0;
    char message[78] = "";
    char stats[78] = "";
    Timer t;
    t.start = true;
    t.count = 0;
    t.limit = 3;

    while(1){
        printf("timer:--%d--\n",t.count);
        printf("system count:--%d--\n",loop_count);
            //--for the case which window is full
            if(nextseqnum<base+WINDOWSIZE){
                send_unreliably(s,sndpkt[nextseqnum],remoteaddr);
                if(base==nextseqnum)
                    t.start = true;
                if(strlen(sndpkt[nextseqnum+1])>1)
                    nextseqnum++;
            }
            else
                printf("window full..\n");
                Sleep(100);

            //--for the case that CLS acknowledge not received properly
            printf("base is %d\n",base);
            if(base == (pkt_count-1)){
                printf("base reaches end, wairing for last ACK(CLS)...\n");
                if(t.count >= t.limit){
                    printf("acquiring CLS timeout, close anyway...");
                    break;
                }
            }

            //--for the case that normal timeout
            if(t.count>=t.limit){
                printf("time out, resending..\n");
                t.count = 0;
                t.start = true;
                timeout_count = base;
                for(;timeout_count<nextseqnum;timeout_count++){
                    send_unreliably(s,sndpkt[timeout_count],remoteaddr);
                }
                printf("resending finished, sleep for 400\n");
                Sleep(100);
            }

            //--receiving process and fsm
            memset(receive_buffer, 0, sizeof(receive_buffer));
            recv_nonblocking(s,receive_buffer, remoteaddr);
            printf("RECEIVE --> %s \n",receive_buffer);
            if(strlen(receive_buffer)>0){
                memset(message,0,sizeof(message));
                crc_rcv = get_crc_op_rest(receive_buffer,message);
                //printf("crc_rcv:%d\n",crc_rcv);
                //crc_cal = (int)CRCpolynomial(message);
                crc_cal = compute_crc_with_newline(message);    //receive_nonblocking() contains the function to get rid of \n
                //printf("crc_cal:%d\n",crc_cal);
                ack_old = ack;
                memset(stats,0,sizeof(stats));
                ack = get_ack_op_stats(message,stats);
                //if ack updated, reset timer
                if(ack>ack_old){
                    t.count = 0;
                    printf("ack updated, timer reset\n");
                }
                //printf("ack:%d\n",ack);
                //printf("stats:%s\n",stats);

                //if cls received, close
                //if base reaches nextseqnum, stop timer
                if(crc_rcv == crc_cal){
                    if(strcmp(stats,"CLS")==0){
                        printf("CLS received, closing...\n");
                        break;
                    }

                    base = ack+1;
                    if(base == nextseqnum){
                        t.start = false;
                        //t.count = 0;
                    }else{
                        t.start = true;
                    }
                }
            }




            if(t.start = true)
                t.count++;
            loop_count++;

    }



//loop ends+++++++++++++++++++++++++++++++++++++++++++++++++++++



    printf("closing everything on the client's side ... \n");
    fclose(fin);
    closesocket(s);

    exit(0);

}

