# makefile for MP5
# Drew Buley
# C20407096
# 11/2024
#
lab5 : bst_mp5.o lab5.o
	gcc -Wall -g bst_mp5.o lab5.o -o lab5 -lm

bst_mp5.o : datatypes.h bst_mp5.c
	gcc -Wall -g -c bst_mp5.c

lab5.o : datatypes.h lab5.c
	gcc -Wall -g -c lab5.c
