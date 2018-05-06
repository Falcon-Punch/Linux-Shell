/*************************************************************************
 File name:  CS460_Shell.c
 Author:     Joseph Schell
 Date:	     03/21/2018
 Class:      Operating Systems
 Assignment: Create a UNIX Shell
 Purpose:    Learn about Unix processes, interprocess communication,
 	     and other shell functions like fork(), exec(), pipe() and
 	     dup2().
 **************************************************************************/
#include "../include/CS460_Shell.h"

/**************************************************************************
 Function:    main
 Description: Driver for the Shell module
 Parameters:  argc	- number of command line arguments
 	      argv	- array of pointers to each argument
 Returned:    EXIT Status
 *************************************************************************/
int main (int argc, char** argv)
{
	int option;
	char* pszCommand;
	pid_t promptPID = getpid ();
	size_t bufferSize = MAX_LEN;
	size_t commandSize;

	option = getopt (argc, argv, "d");
	pszCommand = (char *) calloc (MAX_LEN, sizeof(char));

	while (0 != strcmp (pszCommand, "exit"))
	{
		memset (pszCommand, 0, MAX_LEN);
		printf ("%d> ", promptPID);
		commandSize = getline (&pszCommand, &bufferSize, stdin);

		pszCommand[commandSize - 1] = '\0';

		if ('d' == option && 0 != strcmp (pszCommand, "exit"))
		{
			debugProcessing (pszCommand, commandSize);
		}
		else if (0 != strcmp (pszCommand, "exit"))
		{
			commandSize--;
			normalProcessing (pszCommand, commandSize);
		}
	}

	free (pszCommand);

	return EXIT_SUCCESS;
}

/**************************************************************************
 Function:    	debugProcessing
 Description: 	Shows user input that has been parsed and shows as
 	 	organized information
 Parameters:  	pszCommand - command string input
 	 	strSize - size of input string
 Returned:    	none
 *************************************************************************/
void debugProcessing (char* pszCommand, size_t strSize)
{
	int numBlocks = 0;
	int length, i;
	Block asBlocksArray[MAX_BLOCKS];
	char** hStringTokens = splitString (pszCommand, &length);

	numBlocks = commandToBlocks (hStringTokens, length, asBlocksArray);

	for (i = 0; i < numBlocks; i++)
	{
		printf ("command: %s\n", asBlocksArray[i].pszCommand);
		printf ("\targuments: ");

		if (1 >= asBlocksArray[i].argc)
		{
			printf ("none\n");
		}
		else
		{
			int j;
			for (j = 1; j < asBlocksArray[i].argc; j++)
			{
				printf ("%s ", asBlocksArray[i].paArgv[j]);
			}

			printf ("\n");
		}

		printf ("\tredirection:\n");
		printf ("\t\tstdin: ");

		if (true == asBlocksArray[i].pipeIn)
		{
			printf ("%s", PIPE);
		}
		else if (NULL == asBlocksArray[i].pszStdIn)
		{
			printf ("none");
		}
		else
		{
			printf ("%s", asBlocksArray[i].pszStdIn);
		}

		printf ("\n");
		printf ("\t\tstdout: ");

		if (true == asBlocksArray[i].pipeOut)
		{
			printf ("%s", PIPE);
		}
		else if (NULL == asBlocksArray[i].pszStdOut)
		{
			printf ("none");
		}
		else
		{
			printf ("%s", asBlocksArray[i].pszStdOut);
		}

		printf ("\n");
		printf ("pipe: %s\n", asBlocksArray[i].aPipe);
	}

	printf ("background: %s\n", asBlocksArray[i - 1].aBackground);

	free (hStringTokens);
}

/**************************************************************************
 Function:    	normalProcessing
 Description: 	Shows user input that has been parsed and shows as organized
 	 	information
 Parameters:  	pszCommand - command string input
 	 	strSize - size of input string
 Returned:    	none
 *************************************************************************/
void normalProcessing (char* pszCommand, size_t strSize)
{
	int numBlocks = 0;
	int length;
	Block asBlocksArray[MAX_BLOCKS];
	char** hStringTokens = splitString (pszCommand, &length);

	numBlocks = commandToBlocks (hStringTokens, length, asBlocksArray);

	if (1 == numBlocks)
	{
		singleCommand (asBlocksArray);
	}
	else
	{
		multiCommand (asBlocksArray, numBlocks);
	}

	free (hStringTokens);
}

/**************************************************************************
 Function:    	splitString
 Description: 	splits string into an array of tokens
 Parameters:  	pszCommand - the command string
 	 	pLength - length of the array
 Returned:    	none
 *************************************************************************/
char** splitString (char* pszCommand, int *pArrayLen)
{
	char** hStringTokens = (char**) calloc (MAX_LEN, sizeof(char*));
	char* pszStringToken;
	char aStrDelim[] = " ";
	int counter = 0;

	hStringTokens[counter] = strtok (pszCommand, aStrDelim);
	counter++;

	while (NULL != (pszStringToken = strtok (NULL, aStrDelim)))
	{
		hStringTokens[counter] = pszStringToken;
		counter++;
	}

	*pArrayLen = counter;

	return hStringTokens;
}

/**************************************************************************
 Function:    	commandToBlocks
 Description: 	turns an array of tokens to an array of blocks
 Parameters:  	hStringTokens - address to the array of string tokens
 	 	pLength - length of the array of tokens
 	 	asBlocksArray - array of empty blocks
 Returned:    	number of blocks created
 *************************************************************************/
int commandToBlocks (char** hStringTokens, int arrayLen,
		Block asBlocksArray[])
{
	int blockNum = 0;
	char redirectChar;
	int i = 0;

	while (i < arrayLen)
	{
		asBlocksArray[blockNum].pszCommand = hStringTokens[i];

		if (0 < blockNum && true == asBlocksArray[blockNum - 1].pipeOut)
		{
			asBlocksArray[blockNum].pipeIn = true;
		}
		else
		{
			asBlocksArray[blockNum].pipeIn = false;
		}

		asBlocksArray[blockNum].pipeOut = false;
		asBlocksArray[blockNum].argc = 0;
		asBlocksArray[blockNum].paArgv[asBlocksArray[blockNum].argc] =
				hStringTokens[i];
		asBlocksArray[blockNum].argc++;
		i++;

		while (i < arrayLen && '|' != hStringTokens[i][0] && '&'
					!= hStringTokens[i][0] && ';' != hStringTokens[i][0]
					&& '<' != hStringTokens[i][0] && '>' != hStringTokens[i][0])
		{
			asBlocksArray[blockNum].paArgv[asBlocksArray[blockNum].argc] =
					hStringTokens[i];
			asBlocksArray[blockNum].argc++;
			i++;
		}

		asBlocksArray[blockNum].paArgv[asBlocksArray[blockNum].argc] = NULL;
		asBlocksArray[blockNum].pszStdIn = NULL;
		asBlocksArray[blockNum].pszStdOut = NULL;

		while (i < arrayLen && '|' != hStringTokens[i][0] && '&'
					!= hStringTokens[i][0] && ';' != hStringTokens[i][0]
					&& ('<' == hStringTokens[i][0] || '>' == hStringTokens[i][0]))
		{
			redirectChar = hStringTokens[i][0];
			i++;

			if ('<' == redirectChar)
			{
				asBlocksArray[blockNum].pszStdIn = hStringTokens[i];
			}
			else
			{
				asBlocksArray[blockNum].pszStdOut = hStringTokens[i];
			}

			i++;
		}

		asBlocksArray[blockNum].aPipe = "none";
		asBlocksArray[blockNum].aBackground = "no";

		if (i < arrayLen)
		{
			if ('|' == hStringTokens[i][0] || ';' == hStringTokens[i][0])
			{
				if ('|' == hStringTokens[i][0])
				{
					asBlocksArray[blockNum].pipeOut = true;
					asBlocksArray[blockNum].aPipe = YES;
				}

				i++;
			}
			else if ('&' == hStringTokens[i][0])
			{
				asBlocksArray[blockNum].aBackground = YES;
				i++;
			}
		}
		blockNum++;
	}

	return blockNum;
}

/**************************************************************************
 Function:    	singleCommand
 Description: 	process one command
 Parameters:  	psBlock - pointer to a Block that holds the command
 Returned:	none
 *************************************************************************/
void singleCommand (Block* psBlock)
{
	if (0 == strcmp (psBlock->pszCommand, "cd"))
	{
		changeDirectory (psBlock);
	}
	else
	{
		int fd0, fd1;
		pid_t pid = fork ();

		if (0 > pid)
		{
			perror ("Fork failed");
			exit (1);
		}
		else if (0 == pid)
		{
			if (NULL != psBlock->pszStdIn)
			{
				fd0 = open (psBlock->pszStdIn, O_RDONLY);

				if (0 > fd0)
				{
					perror ("Cannot open file for reading");
					exit (1);
				}

				if (0 != dup2 (fd0, 0))
				{
					perror ("Error: dup2(fd0, 0)");
					exit (1);
				}

				close (fd0);
			}

			if (NULL != psBlock->pszStdOut)
			{
				fd1 = open (psBlock->pszStdOut, O_WRONLY | O_TRUNC | O_CREAT,
						DEFAULT_PERMISSION);

				if (0 > fd1)
				{
					perror ("Cannot open file for writing");
					exit (1);
				}
				if (1 != dup2 (fd1, 1))
				{
					perror ("Error: dup2(fd1, 1)");
					exit (1);
				}

				close (fd1);
			}

			execvp (psBlock->paArgv[0], psBlock->paArgv);
			perror ("The execvp() failed");
			exit (1);
		}
		else
		{
			if (0 != strcmp (psBlock->aBackground, YES))
			{
				waitpid (pid, NULL, 0);
			}
			else
			{
				// Added sleep delay so prompt displays after background
				// process execution. (without this the prompt displays too
				// quickly, before process displays)
				sleep(1);
			}
		}
	}
}

/**************************************************************************
 Function:    	multiCommand
 Description: 	process multiple commands
 Parameters:  	asBlocksArray - array of blocks
 	 	blockNum - number of blocks
 Returned:	none
 *************************************************************************/
void multiCommand (Block asBlocksArray[], int blockNum)
{
  int pipeNum = blockNum - 1;
  int fd[pipeNum][2];
  int i;

  for (i = 0; i < pipeNum; i++)
	{
		if (-1 == pipe (fd[i]))
		{
			perror ("Bad pipe");
			exit (1);
		}
	}

	for (i = 0; i < blockNum; i++)
	{
		execCommand (asBlocksArray + i, i, blockNum, fd);
	}
}

/**************************************************************************
 Function:    	execCommand
 Description: 	execute a command
 Parameters:  	psBlock - pointer to a Block that holds the command
 	 	index - index number of the block
 	 	blockNum - number of blocks
 	 	fd - array of all the file descriptors
 Returned:	none
 *************************************************************************/
void execCommand (Block* psBlock, int index, int blockNum, int fd[][2])
{
	if (0 == strcmp (psBlock->pszCommand, "cd"))
	{
		changeDirectory (psBlock);
	}
	else
	{
		int fd0, fd1;
		pid_t pid = fork ();

		if (0 > pid)
		{
			perror ("Bad fork");
			exit (1);
		}

		if (0 == pid)
		{
			if (NULL != psBlock->pszStdIn)
			{
				fd0 = open (psBlock->pszStdIn, O_RDONLY);
				if (0 > fd0)
				{
					perror ("Cannot open file for reading");
					exit (1);
				}

				if (0 != dup2 (fd0, 0))
				{
					perror ("Error: dup2(fd0, 0)");
					exit (1);
				}

				close (fd0);
			}

			if (NULL != psBlock->pszStdOut)
			{
				fd1 = open (psBlock->pszStdOut, O_WRONLY | O_TRUNC | O_CREAT,
						DEFAULT_PERMISSION);
				if (0 > fd1)
				{
					perror ("Cannot open file for writing");
					exit (1);
				}

				if (1 != dup2 (fd1, 1))
				{
					perror ("Error: dup2(fd1, 1)");
					exit (1);
				}

				close (fd1);
			}

			if (0 == index)
			{
				if (NULL == psBlock->pszStdOut && 0 == strcmp(psBlock->aPipe, YES))
				{
					dup2 (fd[index][1], 1);
				}

				close (fd[index][1]);
				close (fd[index][0]);
			}
			else if (index == blockNum - 1)
			{
				if (NULL == psBlock->pszStdIn)
				{
					dup2 (fd[index - 1][0], 0);
				}
				close (fd[index - 1][1]);
				close (fd[index - 1][0]);
			}
			else
			{
				if (NULL == psBlock->pszStdIn)
				{
					dup2 (fd[index - 1][0], 0);
				}
				if (NULL == psBlock->pszStdOut && 0 == strcmp(psBlock->aPipe, YES))
				{
					dup2 (fd[index][1], 1);
				}

				close (fd[index - 1][1]);
				close (fd[index - 1][0]);
				close (fd[index][1]);
				close (fd[index][0]);
			}

			execvp (psBlock->paArgv[0], psBlock->paArgv);
			perror ("execvp() failed");
			exit (1);
		}

		if (0 == index)
		{
			close (fd[index][1]);
		}
		else if (index == blockNum - 1)
		{
			// Do Not Close!
		}
		else
		{
			close (fd[index][1]);
		}

		if (0 != strcmp (psBlock->aBackground, YES))
		{
			waitpid (pid, NULL, 0);
		}
		else
		{
			// Added sleep delay so prompt displays after background
			// process execution. (without this the prompt displays too
			// quickly, before process displays)
			sleep(1);
		}
	}
}

/**************************************************************************
 Function:    	changeDirectory
 Description: 	implements a change directory command
 Parameters:  	psBlock - pointer to a Block that holds the command
 Returned:	none
 *************************************************************************/
void changeDirectory (Block* psBlock)
{
	char* pDirectory;

	if (1 < psBlock->argc)
	{
		pDirectory = psBlock->paArgv[1];
	}
	else
	{
		pDirectory = getenv ("HOME");
	}

	if (0 > chdir (pDirectory))
	{
		fprintf (stderr, "Cannot move to %s\n", pDirectory);
	}
}
