//159.334 - Networks
///////////////   2015 ////////////////////
// SERVER: prototype for assignment 2.
// This code is different than the one used in previous semesters...
//************************************************************************/
//COMPILE WITH: gcc server_Unreliable_2015.c -o server_Unreliable_2015
//with no losses nor damages, RUN WITH: ./server_Unreliable_2015 1234 0 0
//with losses RUN WITH: ./server_Unreliable_2015 1234 1 0
//with damages RUN WITH: ./server_Unreliable_2015 1234 0 1
//with losses and damages RUN WITH: ./server_Unreliable_2015 1234 1 1
//************************************************************************/
#if defined __unix__ || defined __APPLE__
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#elif defined _WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include "mylib.c"
#endif

#include "UDP_supporting_functions_2015.c"

#define BUFFESIZE 80
//remember, the BUFFESIZE has to be at least big enough to receive the answer from the serv1
#define SEGMENTSIZE 78
//segment size, i.e., if fgets gets more than this number of bytes is segments the message in smaller parts.

//*******************************************************************
//Function to save lines and discarding the header
//*******************************************************************
//You ARE allowed to change this. You will need to alter the NUMBER_OF_WORD_IN_THE_HEADER if you add a CRC
#define NUMBER_OF_WORDS_IN_THE_HEADER 2
void save_line_without_header(char * receive_buffer,FILE *fout){
	char sep[2] = " "; //separation is space
	char *word;
	int wcount=0;
	char lineoffile[BUFFESIZE]="\0";
	for (word = strtok(receive_buffer, sep);word;word = strtok(NULL, sep))
	{
		wcount++;
		if(wcount > NUMBER_OF_WORDS_IN_THE_HEADER){
			strcat(lineoffile,word);
			strcat(lineoffile," ");
		}
	}
	printf("DATA: %s \n",lineoffile);
	lineoffile[strlen(lineoffile)-1]=(char)'\0';//get rid of last space
	if (fout!=NULL) fprintf(fout,"%s\n",lineoffile);
	else {
		printf("error when trying to write...\n");
		exit(0);
	}

}

#if defined __unix__ || defined __APPLE__

#elif defined _WIN32
#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;
#endif

//*******************************************************************
//MAIN
//*******************************************************************
int main(int argc, char *argv[]) {
//********************************************************************
// INITIALIZATION
//********************************************************************
	struct sockaddr_in localaddr,remoteaddr;
#if defined __unix__ || defined __APPLE__
	int s;
#elif defined _WIN32
	SOCKET s;
#endif
	char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
	int n,bytes,addrlen;
	addrlen = sizeof(remoteaddr);
	memset(&localaddr,0,sizeof(localaddr));//clean up the structure
	memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure
	randominit();

#if defined __unix__ || defined __APPLE__

#elif defined _WIN32
//********************************************************************
// WSSTARTUP
//********************************************************************
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}
#endif
//********************************************************************
//SOCKET
//********************************************************************
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s <0) {
		printf("socket failed\n");
	}
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = INADDR_ANY;//server address should be local
	if (argc != 4) {
		printf("2012 code USAGE: server port  lossesbit(0 or 1) damagebit(0 or 1)\n");
		exit(1);
	}
	localaddr.sin_port = htons((u_short)atoi(argv[1]));
	int remotePort=1234;
	packets_damagedbit=atoi(argv[3]);
	packets_lostbit=atoi(argv[2]);
	if (packets_damagedbit<0 || packets_damagedbit>1 || packets_lostbit<0 || packets_lostbit>1){
		printf("2013 code USAGE: server port  lossesbit(0 or 1) damagebit(0 or 1)\n");
		exit(0);
	}
//********************************************************************
//REMOTE HOST IP AND PORT
//********************************************************************
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteaddr.sin_port = htons(remotePort);
	int counter=0;
//********************************************************************
//BIND
//********************************************************************
	if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) {
		printf("Bind failed!\n");
		exit(0);
	}
//********************************************************************
// Open file to save the incoming packets
//********************************************************************
	FILE *fout=fopen("file1_saved.txt","w");
//********************************************************************
//INFINITE LOOP
//********************************************************************

//--init for while
    char tok_buffer[78];
    char rest_buffer[78];
    char pktstat_buff[78];
    char seqnum_buff[78];
    char data_buff[78];

    int expectseqnum = 0;
    int recv_seqnum = 0;

    char pkt_buffer[78];
    memset(pkt_buffer,0,sizeof(pkt_buffer));
    make_pkt_ACK(-1,pkt_buffer);
    //send_unreliably(s,pkt_buffer,remoteaddr);

	while (1) {
//********************************************************************
//RECEIVE
//********************************************************************
        printf("Waiting... \n");

		bytes = recvfrom(s, receive_buffer, SEGMENTSIZE, 0,(struct sockaddr *)(&remoteaddr),&addrlen);

        memset(tok_buffer,0,sizeof(tok_buffer));
        memset(rest_buffer,0,sizeof(rest_buffer));
        memset(pktstat_buff,0,sizeof(pktstat_buff));
        memset(seqnum_buff,0,sizeof(seqnum_buff));
        memset(data_buff,0,sizeof(data_buff));
		n=0;
		while (n<bytes){
			n++;
			if ((bytes < 0) || (bytes == 0)) break;
			if (receive_buffer[n] == '\n') { /*end on a LF*/
				receive_buffer[n] = '\0';
				break;
			}
			if (receive_buffer[n] == '\r') /*ignore CRs*/
				receive_buffer[n] = '\0';
		}
		if ((bytes < 0) || (bytes == 0)) break;
		printf("\n================================================\n");
		printf("RECEIVED --> %s \n",receive_buffer);

        //--my start
        //--check for corrupt
        mytok(receive_buffer,tok_buffer,rest_buffer);
        printf("sys:tok_buffer is:%s;rest_buffer is:%s\n", tok_buffer,rest_buffer);
        int recv_crc, calc_crc;
        recv_crc = atoi(tok_buffer);
        calc_crc = (int)CRCpolynomial(rest_buffer);
        printf("sys:recv_crc is:%d;calc_crc is:%d\n",recv_crc,calc_crc);

        if((recv_crc == calc_crc)&&(recv_crc != 0)){
            //split into stat,#,data
            mytok(rest_buffer,tok_buffer,rest_buffer);
            strcpy(pktstat_buff,tok_buffer);
            mytok(rest_buffer,tok_buffer,rest_buffer);
            strcpy(seqnum_buff,tok_buffer);
            strcpy(data_buff,rest_buffer);


            if(atoi(seqnum_buff) == expectseqnum){
                if(strcmp(pktstat_buff,"PACKET")==0){
                    save_line(data_buff,fout);
                    memset(pkt_buffer,0,sizeof(pkt_buffer));
                    make_pkt_ACK(expectseqnum, pkt_buffer);
                    send_unreliably(s,pkt_buffer,remoteaddr);
                    expectseqnum++;
                }else if(strcmp(pktstat_buff,"CLOSE")==0){
                    memset(pkt_buffer,0,sizeof(pkt_buffer));
                    make_pkt_CLS(expectseqnum, pkt_buffer);
                    send_unreliably(s,pkt_buffer,remoteaddr);
                    break;
                }
            }else{
                printf("redundent send\n");
                send_unreliably(s,pkt_buffer,remoteaddr);
            }
        }else{

        //--FSM default

            printf("default send\n");
            send_unreliably(s,pkt_buffer,remoteaddr);
        }
        //
/*
        //--split receive_buffer into crc_number and PACKET # data
        mytok(receive_buffer,tok_buffer,rest_buffer);
        printf("rest_buffer: %s\n",rest_buffer);
        //--test if crc match
        int recv_crc,calc_crc;
        recv_crc = atoi(tok_buffer);
        calc_crc = (int)CRCpolynomial(rest_buffer);
        printf("syst: recv_crc is %d, calc_crc is %d.\n",recv_crc,calc_crc);
        if(recv_crc == calc_crc){
            printf("syst: crc match...\n");
            if(strncmp(rest_buffer,"PACKET",6)==0){
                printf("syst: received with header PACKET\n");
                sscanf(rest_buffer, "PACKET %d",&recv_seqnum);
                printf("syst: recv_seqnum is:%d\n",recv_seqnum);
                if(recv_seqnum == expectseqnum){
                    //--test--
                    printf("syst: expectseqnum is:%d, match with recv_seqnum:%d\n",expectseqnum,recv_seqnum);

                    //save line, with printf
                    save_line(rest_buffer,fout);

                    //
                    memset(pkt_buffer , 0 , sizeof(pkt_buffer));
                    make_pkt_ACK(expectseqnum , pkt_buffer);

                    send_unreliably(s,pkt_buffer,remoteaddr);
                    expectseqnum++;
                }else{
                    if(expectseqnum>0){
                        send_unreliably(s,pkt_buffer,remoteaddr);
                    }else{
                        printf("syst: no previous ACK sent\n");
                    }
                }
            }
            else if(strncmp(rest_buffer,"CLOSE",5)==0){
                sscanf(rest_buffer, "CLOSE %d", &recv_seqnum);
                if(recv_seqnum == expectseqnum){
                    printf("syst: received CLOSE packet\n");
                    memset(pkt_buffer, 0 , sizeof(pkt_buffer));
                    make_pkt_CLS(expectseqnum , pkt_buffer);
                    send_unreliably(s,pkt_buffer,remoteaddr);
                    expectseqnum++;
                }else{
                    if(expectseqnum>0){
                        send_unreliably(s,pkt_buffer,remoteaddr);
                    }else{
                        printf("syst: no previous ACK sent\n");
                        send_unreliably(s,init_ACK,remoteaddr);
                    }

                }
            }
            else{
                printf("syst: not valid packet\n");
                if(expectseqnum>0){
                    send_unreliably(s,pkt_buffer,remoteaddr);
                }else{
                    printf("syst: no previous ACK sent\n");
                    send_unreliably(s,init_ACK,remoteaddr);
                }
            }
        }else{
            if(expectseqnum>0){
                send_unreliably(s,pkt_buffer,remoteaddr);
                //send ACK expectseqnum-1
            }else{
                printf("syst: no previous ACK sent\n");
                send_unreliably(s,init_ACK,remoteaddr);               //send nothing
            }

        }

/*
		if (strncmp(receive_buffer,"PACKET",6)==0)  {
			sscanf(receive_buffer, "PACKET %d",&counter);
//********************************************************************
//SEND ACK
//********************************************************************
			sprintf(send_buffer,"ACKNOW %d \r\n",counter);
			send_unreliably(s,send_buffer,remoteaddr);
			save_line_without_header(receive_buffer,fout);
		}
		else {
			if (strncmp(receive_buffer,"CLOSE",5)==0)  {//if client says "CLOSE", the last packet for the file was sent. Close the file
			//Remember that the packet carrying "CLOSE" may be lost or damaged!
				fclose(fout);
#if defined __unix__ || defined __APPLE__
				close(s);
#elif defined _WIN32
				closesocket(s);
#endif
				printf("Server saved file1_saved.txt \n");//you have to manually check to see if this file is identical to file1.txt
				exit(0);
			}
			else {//it is not PACKET nor CLOSE, therefore there might be a damaged packet
			//in this assignment, CLOSE always arrive (read UDP_supporting_functions_2012.c to see why...)
			//do nothing, ignoring the damaged packet? Or send a negative ACK? It is up to you to decide.
			}
		}
	}

*/
	}
#if defined __unix__ || defined __APPLE__
	close(s);
#elif defined _WIN32
	closesocket(s);
#endif
	exit(0);
}

