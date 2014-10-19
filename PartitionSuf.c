// PartitionSuf.c
// Partition all suffixes of large collection reads by prefix
//
// Usage: ./PartitionSuf /path/to/*.fasta ./ReadSet_(processor_ID) processor_ID #reads_load_by_each_processor &

//
// Heng Wang
// National University of Defense Technology
// 2014/09/21
//
/////////////////////////////////////////////////////////////

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_LINE_LENGTH 512
#define ALPHABETA_SIZE  4
#define READ_LEN        100
#define READ_PACK_LEN   25
#define BUFF_SIZE       (READ_PACK_LEN + 5) * 100 

#define ConvertBase2Pack(c) c == 'A' ? 0 :(c == 'C'? 1 : (c == 'G'? 2 : (c == 'T'? 3 : 0)))


typedef uint8_t * string;

//uint64_t partitionCount[256] = {0}; //Number of suffixes with specific prefix

inline void String2Pack(char * source, uint8_t * destination, unsigned int len);

/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/




int main(int argc, char ** argv){

    if(argc < 4){
        printf("Usage:  ./partitionTest read.fa outPut_path READ_FILE_ID READS_PER_FILE.\n");
        exit(0);
    }


    uint32_t    i, j, readIndex = 0;
    uint8_t     shift, baseIndex, byteIndex;


    /**********************************************************************************************/

    char    readLine                [MAX_LINE_LENGTH]       = {'A'};
    uint8_t readPack                [MAX_LINE_LENGTH]       = {0};
    uint8_t bwtCode                 [READ_LEN]              = {4}; // 4 represents '$' in BWT Code
    uint8_t readPackEvery4Base   [4][READ_PACK_LEN * 2 - 1] = {0};
    uint8_t S_Prefix                [MAX_LINE_LENGTH];

    uint8_t BUFF[256]               [BUFF_SIZE];
    uint32_t buffIndex[256]      =  {0};

    uint8_t READ_FILE_ID         =  atoi(argv[3]);
    uint32_t READS_PER_FILE      =  atoi(argv[4]);


    FILE *fout[256]; //Partition files

    /**********************************************************************************************/

    for(i = 0; i < 256; i++)
    {
        char bwtFileName    [MAX_LINE_LENGTH] = {};
        char fileNameID     [MAX_LINE_LENGTH] = {};
        char fileNamePrefix [MAX_LINE_LENGTH] = {};
        strcat(bwtFileName, argv[2]);       
        strcat(bwtFileName, "/prefix_");
        sprintf(fileNameID, "%d", i);
        strcat(bwtFileName, fileNameID);
        
        fout[i] = fopen(bwtFileName, "wb");
        if(fout[i] == NULL) {
            fprintf(stderr, "Fail to open file: prefix_%d\n", i);
            exit(1);
        }
        //setvbuf(fout[i], BUFF[i], _IOLBF, BUFF_SIZE);
    }

    /**********************************************************************************************/


    /**********************************************************************************************/

    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL) {
        fprintf(stderr, "%s\n", "Fail to open file");
        exit(0);
    }


    uint32_t globalReadIndex = 0;
    while(fgets(readLine, MAX_LINE_LENGTH, fp))
    {
        if(readLine[0] == '>') continue;
		if(globalReadIndex < READ_FILE_ID * READS_PER_FILE) {globalReadIndex++; continue;}
		if(globalReadIndex  >= (READ_FILE_ID + 1) * READS_PER_FILE) break;
        
        //Convert string to unit8_t pack
        String2Pack(readLine, readPack, READ_LEN + 4);
        for(baseIndex = 1; baseIndex < READ_LEN; baseIndex++)
            bwtCode[baseIndex] = readPack[baseIndex - 1];

        //Pack every 4 bases
        for(j = 0; j < READ_LEN; j++)
        {
            uint8_t tmp = (readPack[j] << 6) + (readPack[j+1] << 4) + (readPack[j+2] << 2) + readPack[j+3];
            readPackEvery4Base[j%4][j>>2] = tmp;
            //partitionCount[tmp]++;
        }

        //Write suffixes to different prefix-files
        for(shift = 0; shift < 4; shift++)
        {
            for(byteIndex = 0; byteIndex < READ_PACK_LEN; byteIndex++)
            {
                uint8_t value = readPackEvery4Base[shift][byteIndex] ;
                uint16_t baseIndex = (byteIndex << 2) + shift ;
                uint8_t suffixIndex[5];

                suffixIndex[0] = baseIndex;
                //uint32_t globalReadIndex = readIndex + READ_FILE_ID * READS_PER_FILE;
                suffixIndex[1] = (globalReadIndex >> 24 ) % 256;
                suffixIndex[2] = (globalReadIndex >> 16 ) % 256;
                suffixIndex[3] = (globalReadIndex >> 8  ) % 256;
                suffixIndex[4] = (globalReadIndex       ) % 256;
                
                fwrite(bwtCode + baseIndex, sizeof(uint8_t), 1, fout[value]);
                fwrite(readPackEvery4Base[shift] + byteIndex + 1, sizeof(uint8_t), READ_PACK_LEN - 1, fout[value]);
                fwrite(suffixIndex, sizeof(uint8_t), 5, fout[value]);
                
                /*
                BUFF[value][buffIndex[value]++] = bwtCode[baseIndex];
                memcpy(BUFF[value] + buffIndex[value], readPackEvery4Base[shift] + byteIndex + 1, READ_PACK_LEN - 1);
                buffIndex[value] += READ_PACK_LEN - 1;
                BUFF[value][buffIndex[value]++] = baseIndex;
                BUFF[value][buffIndex[value]++] = (readIndex >> 24 ) % 256;
                BUFF[value][buffIndex[value]++] = (readIndex >> 16 ) % 256;
                BUFF[value][buffIndex[value]++] = (readIndex >>  8 ) % 256;
                BUFF[value][buffIndex[value]++] = (readIndex       ) % 256;

                if(buffIndex[value] == BUFF_SIZE){
                    fwrite(BUFF[value], sizeof(uint8_t), BUFF_SIZE, fout[value]);
                    buffIndex[value] = 0;
                }
                */
            }
        }

        globalReadIndex++;
    }

    /**********************************************************************************************/
    
    for(i = 0; i < 256; i++)
        fclose(fout[i]);

    fclose(fp);

    /**********************************************************************************************/
    /*
    FILE *partition = fopen("partitionCount.ini", "wb");
    if(partition == NULL) {
        fprintf(stderr, "%s\n", "Fail to open file: partitionCount.ini");
        exit(0);
    }
    fwrite(partitionCount,sizeof(uint64_t), 256, partition);
    fclose(partition);
    */
    /**********************************************************************************************/
    //printf("%u\n", readIndex);
    return 0;
}

inline void String2Pack(char * source, uint8_t * destination, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        destination[i] = ConvertBase2Pack(source[i]);
    }
}

