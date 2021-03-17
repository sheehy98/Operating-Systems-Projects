~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PROJECT 4 - Hamayel Qureshi & Mago Sheehy ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We succesfully completed both problems with all requirements fulfilled. We also feel that the way we print information
to the console is especially useful in determining what is going in our programs in both problem1 (spectacular.c) and
problem2 (fedoops.c).

--------------- Data Structures:

Almost all of our information is stored in structs. In situations where we needed to implement a queue (the pile of pending
packages or worker queues) we implemented a linked list of package/worker structs. In other situations where a strict queue
was not necessary, we simply implemented global arrays of structs. In one situation, we have an array of linked lists in problem 
2 - an array representing each team's worker queue. Our code comments are extensive and go into depth about how we implemented 
our solutions and data structures if you would like to refer to those.

--------------- Problem 1:

In problem1 (spectacular), we actually print out the stage with each of the stage positions either being empty, or occupied
with the type (flamenco dancer/juggler/soloist) and its ID number such that it prints something out like:

[ JGLR #05 ________ JGLR #06 ________ ] or [ SOLO #01 ________ ________ ________ ]. The underscores represent an empty stage
position, and the order of the stage positions is as follows: Position Zero, One, Two, and Three.

Please also keep in mind the following abbreviations and their meanings: 

JGLR = juggler 
FDAN = flamenco dancer
SOLO = soloist

JOIN = signifies a performer joining the stage (then prints out stage + relevant information about who joined and their performance time)
DONE = signifies a performer leaving the stage (then prints out the stage + relevant info about who left)

--------------- Problem 2:

In problem2, we developed abbreviations/codes that signify what each worker is doing as it works through the pile of packages. 
Because we print out so much information (which we believe is relevant and something a real life warehouse automation company
may like to see), the output for each run is around two thousand lines long. This is also because there are 80 packages
to process, with each package having between 1 and 4 stations to be processed through.

The codes/abbreviations and their meanings are as follows:

BUSY: indicates that the worker cannot work on its package yet, because the team already has a worker working on a package
GRAB: self explanatory, signifies that a worker has grabbed a package
STRT: means start, indicates that a worker has started to work on a package
INST: means number of instructions for the package, the following information will tell the user how many instructions the package has
INFO: the array of instructions that a package has attached to it (in order)
MOVE: signifies that the worker is /attempting/ to move the package across a conveyor belt and its destination
WAIT: indicates that the station that the worker is attempting to move a package to is currently busy with a package and it must wait
WORK: indicates that a station was free, and that the worker is working on their package at that station
VLNT: indicates that a package is being shaken violently if it is fragile and if the station is the 'jostle' station
DONE: signifies that a worker is done working on a package at a particular station
FREE: indicates that the station is now free for use
CMLT: indicates that the worker is completely done with a package


--------------- More:

We feel that the extra implementation of printing exactly what each thread is doing and helping visualize precisely how the program is 
running in both problem 1 and 2 is deserving of a few extra points :^)

--------------- Should Know:

We had a weird situation where our program is running very fairly on both our virtual machines. However, when we run it on WSL2 (Ubuntu 16.04)
(which is actually just a Ubuntu 16.04 VM without a GUI) where we've developed the rest of our projects (and problem 2) without any issues, the program 
is not fair at all. We messed around with our random 'performance' times, made them both longer and shorter on the WSL2 machine and neither seemed to work.
Because the professor mentioned all submissions are graded on a Ubuntu 16.04 virtual machine, we stuck to testing it on VirtualBox with Ubuntu 16.04 where
it runs perfectly as expected.