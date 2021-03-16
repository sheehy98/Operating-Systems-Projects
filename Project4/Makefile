all: fedoops.c fedoops.h spectacular.c spectacular.h
	gcc -pthread -o fedoops fedoops.c
	gcc -pthread -o spectacular spectacular.c 

fedoops: fedoops.c fedoops.h
	gcc -pthread -o fedoops fedoops.c 

spectacular: spectacular.c spectacular.h
	gcc -pthread -o spectacular spectacular.c 

clean: 
	rm -f *.o
	rm -f fedoops
	rm -f spectacular