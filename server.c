#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#include "UDP_supporting_functions_2015.c"
#include "sctbase.c"

#define BUFFESIZE 80
#define SEGMENTSIZE 78
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

#define WSVERS MAKEWORD(2,0)
WSADATA wsadata;

int main(int argc, char *argv[]) {
	struct sockaddr_in localaddr,remoteaddr;

	SOCKET s;

	char receive_buffer[BUFFESIZE];
	int n,bytes,addrlen;
	addrlen = sizeof(remoteaddr);
	memset(&localaddr,0,sizeof(localaddr));//clean up the structure
	memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure
	randominit();

	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}

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

	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteaddr.sin_port = htons(remotePort);
	int counter=0;

	if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) {
		printf("Bind failed!\n");
		exit(0);
	}




//start++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    int crc_rcv=0;
    int crc_cal = 0;
    char message[78]="";
    char header[78]="";
    int seqnum = 0;
    int expectseqnum = 0;
    char data[78]="";
    char send_buffer[BUFFESIZE] = "ACK -1 ";


	FILE *fout=fopen("file1_saved.txt","w");
	while (1) {
        printf("\n================================================\n");
        printf("Waiting... \n");
		bytes = recvfrom(s, receive_buffer, SEGMENTSIZE, 0,(struct sockaddr *)(&remoteaddr),&addrlen);
        printf("Received %d bytes\n",bytes);

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
		printf("RECEIVED --> %s \n",receive_buffer);

        memset(message,0,sizeof(message));
        memset(header,0,sizeof(header));
        memset(data,0,sizeof(data));
        crc_rcv = 0;
        crc_rcv = get_crc_op_rest(receive_buffer,message);
        crc_cal = 0;
        crc_cal = compute_crc_with_newline(message);
        seqnum = 0;
        seqnum = get_seqnum_op_header_data(message,header,data);
        //printf("crc_rcv-%d-\nheader-%s-\nseqnum-%d-\ndata-%s\n",crc_rcv,header,seqnum,data);

        if(crc_rcv==crc_cal){
            if(seqnum==expectseqnum){
                if(strcmp(header,"PACKET")==0){
                    save_line(data,fout);
                    memset(send_buffer,0,sizeof(send_buffer));
                    make_pkt(seqnum,"ACK","\n",send_buffer);
                    send_unreliably(s,send_buffer,remoteaddr);
                    expectseqnum++;
                }else if(strcmp(header,"CLOSE")==0){
                    memset(send_buffer,0,sizeof(send_buffer));
                    make_pkt(seqnum,"CLS","\n",send_buffer);
                    send_unreliably(s,send_buffer,remoteaddr);
                    printf("file recv finished, closing...\n");
                    //Sleep(2000);
                    break;
                }
            }
            else{
                printf("seqnum not expected\n");
                send_unreliably(s,send_buffer,remoteaddr);
            }
        }else{
            printf("crc not match\n");
            send_unreliably(s,send_buffer,remoteaddr);
        }

/*
		if (strncmp(receive_buffer,"PACKET",6)==0)  {
			sscanf(receive_buffer, "PACKET %d",&counter);
			sprintf(send_buffer,"ACKNOW %d \r\n",counter);
			send_unreliably(s,send_buffer,remoteaddr);
			save_line_without_header(receive_buffer,fout);
		}
		else {
			if (strncmp(receive_buffer,"CLOSE",5)==0)  {
                fclose(fout);
				closesocket(s);
				printf("Server saved file1_saved.txt \n");//you have to manually check to see if this file is identical to file1.txt
				exit(0);
			}
			else {
                //printf("send nothing\n");
			    memset(send_buffer,0,sizeof(send_buffer));
                sprintf(send_buffer,"hehehe %s", receive_buffer);
                send_unreliably(s,send_buffer,remoteaddr);//it is not PACKET nor CLOSE, therefore there might be a damaged packet
			//in this assignment, CLOSE always arrive (read UDP_supporting_functions_2012.c to see why...)
			//do nothing, ignoring the damaged packet? Or send a negative ACK? It is up to you to decide.
			}
		}
*/
	}
//end+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	closesocket(s);
	exit(0);
}

