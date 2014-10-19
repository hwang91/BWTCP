// prefixCount.c
// Count number of suffixes with specific prefix
//
// Usage: ./prefixCount read.fa read_length partitionCount.ini
//
// Heng Wang
// National University of Defense Technology
// 2014/09/18
//
/////////////////////////////////////////////////////////

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

#define ConvertBase2Pack(c) c == 'A' ? 0 :(c == 'C'? 1 : (c == 'G'? 2 : (c == 'T'? 3 : 0)))

int READ_LEN;

unsigned long long partitionCount[256] = {0}; //Number of suffixes with specific prefix

inline void String2Pack(char * source, uint8_t * destination, unsigned int len);

int main(int argc, char ** argv){

    if(argc != 4){
        printf("\n\tUsage: ./prefixCount read.fa read_length outputFile\n\n");
        exit(0);
    }

    READ_LEN = atoi(argv[2]);

    unsigned long long i = 0, j;

    ///////////////////////////////////////////////////////////////////
    //  Read seuquence and convert ACGT to 0123 respetively. 
    ///////////////////////////////////////////////////////////////////
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL) {
        fprintf(stderr, "Fail to open file: %s\n", argv[1]);
        exit(0);
    }


    char readLine[MAX_LINE_LENGTH];
    uint8_t readPack[MAX_LINE_LENGTH];
    //fgets(readLine, MAX_LINE_LENGTH, fp);
    while(fgets(readLine, MAX_LINE_LENGTH, fp)){
        if(readLine[0] == '>') continue;
        
        for(j = READ_LEN; j < READ_LEN + 4; j++) readLine[j] = 'A';
        String2Pack(readLine, readPack, READ_LEN+4);
        
        for(j = 0; j < READ_LEN; j++){
            partitionCount[readPack[j]*64 + readPack[j+1]*16 + readPack[j+2]*4 + readPack[j+3]]++;
        }
    }
    fclose(fp);
    /*
    for(i = 0; i < 256; i++){
        printf("Prefix %4u num: %llu\n", (unsigned)i, partitionCount[i]);
    }
    */
    ///////////////////////////////////////////////////////////////////
    //  List star position of suffixes with particular 4-prefix
    ///////////////////////////////////////////////////////////////////
     
    FILE *partition = fopen("partitionCount.ini", "wb");
    if(partition == NULL) {
        fprintf(stderr, "%s\n", "Fail to open file: partitionCount.ini");
        exit(0);
    }
    fwrite(partitionCount,sizeof(unsigned long long), 256, partition);
    fclose(partition);
	fprintf(stderr," [MESSAGE] prefixCount Done. \n");
    return 0;
}

inline void String2Pack(char * source, uint8_t * destination, unsigned int len){
    int i;
    for(i = 0; i < len; i++){
        destination[i] = ConvertBase2Pack(source[i]);
    }
}



