#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <openssl/aes.h>
#include <unistd.h>
#include <string.h>

static const uint8_t rsbox[256] = {
  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

char *timingFileName;
char *cipherFileName;
char *resultFileName;
int threshold = 2000;
int key[16][256] = {0};

FILE *timingFP, *cipherFP, *resultFP;

uint8_t *ciphertext;
uint32_t *timing;

unsigned long long counter[16][256];
float times[16][256];

void printHelp(char* argv[]);
void parseOpt(int argc, char *argv[]);
void init();
void finish();
bool loadTrace();
void processTrace();
void saveResult();

int main(int argc, char** argv){
    parseOpt(argc, argv);
    init();
#ifdef DEBUG
    uint32_t i = 0;
#endif

	while(loadTrace() == true){
        //printf("1");
        processTrace();
#ifdef DEBUG
        i++;
#endif
	}
    saveResult();
    finish();
#ifdef DEBUG
        printf("Number samples used: %i\n", i);
#endif
}


int findMaxIndex(int arr[]){
	int max = 0;
	for (int i = 1; i < 255 ; i++){
		if (arr[i] > arr[max]){
			max = i;
		}
	}
	return max;
}
void saveResult()
{
    fprintf(resultFP, "%x \n", findMaxIndex(key[1]));
    fprintf(resultFP, "%x \n", findMaxIndex(key[5]));
    fprintf(resultFP, "%x \n", findMaxIndex(key[9]));
    fprintf(resultFP, "%x \n", findMaxIndex(key[13]));
}
void processTrace()
{
    //int counter = 0;
    int i;
	for(i = 0; i < 16; i++){
		if(*timing < threshold){
            for (int k = 0; k < 256; k++){
                if ((int)rsbox[k ^ ciphertext[i]] < 16){
                    //printf("in\n");
                    //printf("before: %d\n", key[(int)rsbox[k ^ ciphertext[i]]][k]);
                    //key[(int)rsbox[k ^ ciphertext[i]]][k]++;
                    key[i][k]++;
                    //printf("after: %d\n", key[(int)rsbox[k ^ ciphertext[i]]][k]);
                    //counter++;
                }
            }
			//counter[i][ciphertext[i] & 0xff] += 1;
			//times[i][ciphertext[i] & 0xff] += *timing;
        }
	}
    //printf("counter = %d\n", counter);
}
bool loadTrace()
{
	return fread(timing, sizeof(uint32_t), 1, timingFP) &&
            fread(ciphertext, sizeof(uint8_t), 16, cipherFP);
}
void resetCounter()
{
    int i, j;
	for(i = 0; i < 16; i++){
		for(j = 0; j < 256; j++){
			counter[i][j] = 0;
		}
	}
}
void init()
{
    /* allocate memory for one trace */
    ciphertext = (uint8_t *) malloc(sizeof(uint8_t) * 16);
    timing = (uint32_t *) malloc(sizeof(uint32_t));

    /* open required file to read/write */
    timingFP = fopen(timingFileName, "rb");
    cipherFP = fopen(cipherFileName, "rb");
    resultFP = fopen(resultFileName, "w");
    resetCounter();
}
void finish()
{
   free(ciphertext);
   free(timing);

   fclose(timingFP);
   fclose(cipherFP);
   fclose(resultFP);
}

void printHelp(char* argv[])
{
    fprintf(
            stderr,
            "Usage: %s [-t timing file name] "
            "[-c cipher file name] "
            "[-o result file name] "
            "[-s threshold (default: 150)] "
            "\n",
            argv[0]
           );
    exit(EXIT_FAILURE);
}

void parseOpt(int argc, char *argv[])
{
    int opt;
    while((opt = getopt(argc, argv, "c:t:o:s:")) != -1){
        switch(opt){
        case 'c':
            cipherFileName = optarg;
            break;
        case 't':
            timingFileName = optarg;
            break;
        case 'o':
            resultFileName = optarg;
            break;
        case 's':
            threshold = atoi(optarg);
            break;
        default:
            printHelp(argv);
        }
    }
    if(timingFileName == NULL){
        printHelp(argv);
    }
    if(cipherFileName == NULL){
        printHelp(argv);
    }
    if(resultFileName == NULL){
        printHelp(argv);
    }
}