# makefile for MP4
# Version: 1
#
# -lm is used to link in the math library
#
# -Wall turns on all warning messages 
#
comp = gcc
comp_flags = -g -Wall 
comp_libs = -lm  

lab4 : linked_list_lib.o mem.o lab4.o
	$(comp) $(comp_flags) linked_list_lib.o mem.o lab4.o -o lab4 $(comp_libs)

linked_list_lib.o : linked_list_lib.c datatypes.h linked_list_lib.h
	$(comp) $(comp_flags) -c linked_list_lib.c

mem.o : mem.c mem.h
	$(comp) $(comp_flags) -c mem.c

lab4.o : lab4.c datatypes.h linked_list_lib.h mem.h
	$(comp) $(comp_flags) -c lab4.c

clean :
	rm -f *.o lab4 core

