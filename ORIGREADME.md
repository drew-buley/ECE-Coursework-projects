# ECE-Coursework-projects
Projects from ECE 2220 - Data Structures and Algorithms. Taken Fall of 2024
=======
# mp6
MP6: Hash table and Two Sums

See the mp6.pdf found on canvas for details about the assignment requirements.

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

   gh repo clone clemson-ece-2230/mp6-more details

In a terminal on your Ubuntu machine, navigate to a mp6 folder and paste 
the clone command.  Now you will have template files for the assignment.

To submit

In the directory with your code do (change file names as needed):

    git add lab6.c table.c table.h 
    git add hashes.c hashes.h makefile
    git add mytestplan myperformance
    git commit -m "final mp6 code"
    git push

(you probably will not make any changes to hashes.c, hashes.h, or makefile)
