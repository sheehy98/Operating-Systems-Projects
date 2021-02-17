~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PROJECT 2 - Hamayel Qureshi & Mary Braen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This project was relatively straightforward. The only section we struggled with a little was in implementing policy
analysis for round robin. However, we were able to solve our issues after implementing a second linkedlist for round
robin jobs specifically in order to keep track of the response and turnaround time (wait time was simple: tTime - length).
Learned more about pointers as we attempted to clean the code up with a header file (passing in direct references of a variable,
instead of making a local copy in the function).

DS&A:

As discussed, the only data structure we used was a linked list (two of them, one for regular jobs, and one for running RR,
jobs which we called "completedJobs" which may be a misnomer but ¯\_(ツ)_/¯). 

FIFO was easy, as our method of adding a job to the linked list was to append it, instead of adding to the start of the 
linked list. 

SJF was a simple implementation of bubble sort on a linked list, then printing out the linked list.

RR: after implementing RR, we found out our implementation is a bastardization/twist of the actual RR algorithm. Nice.

Analysis:

Analysis for FIFO and SJF was exactly the same. Looped through the nodes in the linkedlist, added up the response and 
turnaround times, averaged them then printed.

Analysis for SJF depended on the second linked list we added. After each job is run for its alloted time slice, check 
if job exists in second list -> edit node in list, if not -> add node to list.