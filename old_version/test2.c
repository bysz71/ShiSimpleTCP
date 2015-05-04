#include <stdlib.h>
#include <stdio.h>
#include "mylib.c"
int main(){
    /*
    char arr[10][10];
    char temp[10] = "aaaa";
    strcpy(arr[10],temp);
    printf("%s\n",arr[10]);
    */

    FILE *fin = fopen("file1.txt","r");
    if(fin==NULL){
        printf("cannot open file\n");
        exit(0);
    }

    char chararray[100][78];
    int n = 0;
    for(;n<100;n++){
        memset(chararray[n],0,sizeof(chararray[n]));
    }
    buffer_file_to_array(chararray,fin);
    array_pktize(chararray);

}
