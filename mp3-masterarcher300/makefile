# The makefile for MP3.
# Type:
#   make         -- to build program lab3
#   make design  -- check for simple design errors (incomplete)
#   make driver  -- to compile testing program
#   make clean   -- to delete object files, executable, and core
#
# You should not need to change this file.  
#
# Format for each entry
#    target : dependency list of targets or files
#    <tab> command 1
#    <tab> command 2
#    ...
#    <tab> last command
#    <blank line>   -- the list of commands must end with a blank line

lab3 : linked_list_lib.o crowd_support.o lab3.o
	gcc -Wall -g linked_list_lib.o crowd_support.o lab3.o -o lab3

linked_list_lib.o : linked_list_lib.c datatypes.h linked_list_lib.h
	gcc -Wall -g -c linked_list_lib.c

crowd_support.o : crowd_support.c datatypes.h linked_list_lib.h crowd_support.h
	gcc -Wall -g -c crowd_support.c

lab3.o : lab3.c datatypes.h linked_list_lib.h crowd_support.h
	gcc -Wall -g -c lab3.c

driver : driver.o linked_list_lib.o crowd_support.o
	gcc -Wall -g linked_list_lib.o crowd_support.o driver.o -o driver

driver.o : driver.c datatypes.h linked_list_lib.h crowd_support.h
	gcc -Wall -g -c driver.c

#  @ prefix suppresses output of shell command
#  - prefix ignore errors
#  @command || true to avoid Make's error
#  : is shorthand for true
design :
	@grep -e "-> *llhead" lab3.c crowd_support.c ||:
	@grep -e "-> *lltail" lab3.c crowd_support.c ||:
	@grep -e "-> *llcount" lab3.c crowd_support.c ||:
	@grep -e "-> *llsortedstate" lab2.c crowd_support.c ||:
	@grep -e "-> *rightlink" lab3.c crowd_support.c ||:
	@grep -e "-> *leftlink" lab3.c crowd_support.c ||:
	@grep -e "-> *userdata" lab3.c crowd_support.c ||:
	@grep "llnode_t" lab3.c crowd_support.c ||:
	@grep "department_id" linked_list_lib.c ||:
	@grep "username" linked_list_lib.c ||:
	@grep "ip_address" linked_list_lib.c ||:
	@grep "time_received" linked_list_lib.c ||:
	@grep "order_received" linked_list_lib.c ||:
	@grep "ticket_number" linked_list_lib.c ||:
	@grep "crowd_" linked_list_lib.c ||:

clean :
	rm -f *.o lab3 driver core a.out grading*

