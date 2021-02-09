~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PROJECT 1 - Hamayel Qureshi~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Overall, this project went pretty smoothly. I had a few hangups due to having to relearn C after a long hiatus.
Most of my programming for the last 2 years has been done in Python and JavaScript, so please excuse me if some
of my C looks rough - the entirety of my experience with C thus far has been writing embedded code.

Please note, I did not include any helper or header files as I wish to keep number of files I am submitted and
the associated makefile as small as possible. For future projects, I may choose to refactor commonly used code
into helper and header files.

I did not use any data structures worth mentioning in this project. I kept it simple with arrays where required,
though in a few places I wish I had taken the time to implement a linked list instead of a simple array. Next time!
I left plenty of comments throughout my code as I wrote it to explain my thinking.

Testing for the project was simple, no scripting. I ran my programs with a variety of different seed values store in
txt files. I ran programs repeatedly to catch bugs (of which there were a few due to silly things like assuming int 
defaults to 0). I ran my programs in Ubuntu 16.04 on Windows Subsystem for Linux and a Virtual Machine.

Prolific: Works as expected. The assignment did not explictly mention we could use the sleep() function but Guin
informed me that it would be fine to make the child sleep for a set amount of time before exiting.

Generation: Works as expected. I use exit() to pass the new descendant count to the parent process, which I reco
gnize isn't the ideal way of doing things based on some readings. In the future, I should implement pipes,
however I wasn't entirely sure I understood them well enough. Will do for future projects. The forking process is
in a recursive function called "descend()".

Explorer: Works as expected. Good learning process into how exec() works.

Slug: Works as expected. Very straightforward. However, I added an exit(0) for the waitpid in slugrace to recognize 
the process is over.

Slugrace: Had a lot of trouble with this one, specifically in figuring out how to interpret the 0.25 second delay 
and printing of running processes requirement. Originally, I had two different for loops - for my final submission,
I have replaced the latter for loop with a while loop (with another for loop inside it), which allows me to run the
process mostly correctly. I check if children have not finished by using waitpid, and seeing if the return value is 
less 1 (this is what I added an exit(0) to slug for); then, I print the currently running children (which are stored
in an array, alongside an array of child processes that have already ran which helps the program decide which processes
to print). Two potential issues in my slugrace code: 1) there is a +.25 bias added to the time readings, perhaps due 
to my usleep placement. I am unsure if this expected or not, however I simply subtract by .25 when calculating time to 
compensate, which I hope is okay. 2) My function that prints the currently running children doesn't perfectly match the
output example in the project description; I'm not sure if this is an issue or not, but it seems to work fine.
