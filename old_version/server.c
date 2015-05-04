#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include "mylib.c"
#include "UDP_supporting_functions_2015.c"

#define BUFFESIZE 80
#define SEGMENTSIZE 78
#define NUMBER_OF_WORDS_IN_THE_HEADER 2

//extract message data from line, save into text file
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

    //--initialization
    struct sockaddr_in localaddr,remoteaddr;
    SOCKET s;
    char send_buffer[BUFFESIZE],receive_buffer[BUFFESIZE];
	int n,bytes,addrlen;

	addrlen = sizeof(remoteaddr);
	memset(&localaddr,0,sizeof(localaddr));//clean up the structure
	memset(&remoteaddr,0,sizeof(remoteaddr));//clean up the structure
	randominit();

    //--wsa startup--
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		WSACleanup();
		printf("WSAStartup failed\n");
	}

	//--socket--
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

	//--remote host ip and port--
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteaddr.sin_port = htons(remotePort);
	int counter=0;

	//--bind--
	if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) != 0) {
		printf("Bind failed!\n");
		exit(0);
	}

	//--open file to save
	FILE *fout=fopen("file1_saved.txt","w");

    int expectedsenum = 0;
	while (1) {
        //--receive
		bytes = recvfrom(s, receive_buffer, SEGMENTSIZE, 0,(struct sockaddr *)(&remoteaddr),&addrlen);
		printf("Received %d bytes\n",bytes);

        //process received pkt
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

/////////
        //extract crc in integer
        char tok[80] = "";
        char rest[80] = "";
        mytok(receive_buffer , tok ,rest);
        int crc;
        crc = atoi(tok);
        //compute crc for rest
        int crc_compute;
        crc_compute = (int)CRCpolynomial(rest);



/////////
		if (strncmp(receive_buffer,"PACKET",6)==0)  {
			//put packet number into 'counter'
			sscanf(receive_buffer, "PACKET %d",&counter);
            //put 'ACK' into send_buffer
            sprintf(send_buffer,"ACKNOW %d \r\n",counter);
			//send 'ACK'
			send_unreliably(s,send_buffer,remoteaddr);
			save_line_without_header(receive_buffer,fout);
		}
		else {
			if (strncmp(receive_buffer,"CLOSE",5)==0)  {//if client says "CLOSE", the last packet for the file was sent. Close the file
			//Remember that the packet carrying "CLOSE" may be lost or damaged!
				fclose(fout);
				closesocket(s);
				printf("Server saved file1_saved.txt \n");//you have to manually check to see if this file is identical to file1.txt
				exit(0);
			}
			else {//it is not PACKET nor CLOSE, therefore there might be a damaged packet
			//in this assignment, CLOSE always arrive (read UDP_supporting_functions_2012.c to see why...)
			//do nothing, ignoring the damaged packet? Or send a negative ACK? It is up to you to decide.
			}
		}
	}
	closesocket(s);
	exit(0);
}

int getcrc(char* recv_buffer){
    char* temp;
    temp = strtok(recv_buffer, " ");
    int crc;
    crc = atoi(temp);
}

char* getrest(char* recv_buffer){
    char temp[80];
    strcpy(temp,recv_buffer);

    char* tok = strtok(temp," ");
    int len;
    len = strlen(tok)+1;

    char* result;
    result = substring(recv_buffer,len);
    return result;
}

char* substring(char* source, int start){
    char temp[80];
    int n;
    int len = strlen(source) - start;
    for(n = 0; n < len; n++){
        temp[n] = source[start+n];
    }
    temp[n+1] = '\0';
    printf("substring output is: %s\n",temp);
    return temp;
}
