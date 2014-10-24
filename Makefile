all:	user SS LS
	
user:	user.c
	gcc -ggdb user.c -o user

SS:	SS.c
	gcc -ggdb SS.c -o SS
	
LS:	LS.c
	gcc -ggdb LS.c -o LS	
clean:	
	rm -f user LS SS *.o