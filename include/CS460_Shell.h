/*************************************************************************
 File name:  CS460_Shell.h
 Author:     Joseph Schell
 Date:	     03/21/2018
 Class:      Operating Systems
 Assignment: Create a UNIX Shell
 Purpose:    Header file for the UNIX Shell Module
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

//*************************************************************************
// 				Constants
//*************************************************************************
#define MAX_LEN 2048
#define MAX_BLOCKS 512
#define ARGV_SIZE 32
#define DEFAULT_PERMISSION 0644
#define PIPE "PIPE"
#define YES "YES"

//*************************************************************************
// 			    User-defined types
//*************************************************************************
struct Block
{
	char* pszCommand;
	char* paArgv[ARGV_SIZE];
	char* pszStdIn;
	char* pszStdOut;
	char* aPipe;
	char* aBackground;
	int argc;
	int pipeIn;
	int pipeOut;
};

typedef struct Block Block;

//*************************************************************************
// 			    Function Prototypes
//*************************************************************************
void debugProcessing (char* pszCommand, size_t strSize);
void normalProcessing (char* pszCommand, size_t strSsize);
char** splitString (char* pszCommand, int* pArrayLen);
int commandToBlocks (char** hStringTokens, int arrayLen,
		Block asBlocksArray[]);
void singleCommand (Block* psBlock);
void multiCommand (Block asBlocksArray[], int blockNum);
void execCommand (Block* psBlock, int index, int blockNum, int fd[][2]);
void changeDirectory (Block* psBlock);
