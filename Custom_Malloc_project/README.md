[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/z3pBG2le)
# mp4
Dynamic Memory Management

See the mp4.pdf found on canvas for details about the assignment requirements.

You must first clone the template code.  

To clone and then later submit, review the Quiz1 details on Canvas about how 
to use gh and git.

You may need to login if you find you get errors about permissions:

    gh auth login 

To check if you are loggged in use 

    gh auth status

To clone

On the github classroom page with the code look for the green "<> Code"
button.  Select the "GitHub CLI" tab.  Use the copy icon to get the line that
starts with (this is incomplete):

   gh repo clone clemson-ece-2230/mp4-more details

In a terminal on your Ubuntu machine, navigate to a mp4 folder and paste 
the clone command.  Now you will have template files for the assignment.

To submit

In the directory with your code do:

    git add lab4.c mem.c linked_list_lib.c 
    git add datatypes.h mem.h linked_list_lib.h
    git add makefile mytestplan myperformance

(you can name the files mytestplan and myperformance files anything you like. 
Do not submit other files)

    git commit -m "final mp4 code"

(You can make the message anything you like.)

    git push

Notice you are not adding any of my testing scripts or testing files.  Just
the code needed to compile and run your program with your test script.

