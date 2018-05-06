#****************************************************************************
# File name:  Makefile
# Author:     Joseph Schell
# Date:       03/21/2018
# Class:      CS460 - Operating Systems
# Assignment: Create a UNIX Shell
# Purpose:    Creates the UNIX Shell module
#****************************************************************************/

CC = gcc
CFLAGS = -g -Wall -lm
OBJECTS = bin/CS460_Shell.o
COMMANDS = ./CS460_Shell

.PHONY: all debug clean valgrind valgrind_debug tarball

all: CS460_Shell bin/CS460_Shell.o

CS460_Shell: ${OBJECTS}
	${CC} ${CFLAGS} -o CS460_Shell ${OBJECTS}
	
bin/CS460_Shell.o: src/CS460_Shell.c include/CS460_Shell.h
	${CC} ${CFLAGS} -o bin/CS460_Shell.o -c src/CS460_Shell.c

debug: CS460_Shell
	${COMMANDS} -d
	
clean:
	rm -f CS460_Shell ${OBJECTS}

valgrind: CS460_Shell
	valgrind -v --leak-check=yes --track-origins=yes \
	--leak-check=full --show-leak-kinds=all ${COMMANDS}
	
valgrind_debug: CS460_Shell
	valgrind -v --leak-check=yes --track-origins=yes \
	--leak-check=full --show-leak-kinds=all ${COMMANDS} -d
	
tarball: clean
	tar czf ../CS460_Shell_sche5694.tar.gz ../CS460_Shell_sche5694
