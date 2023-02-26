all : compile

compile :
	gcc -g -Wall -c -o lib.o lib.c
	gcc -g -Wall -c -o pwd.o pwd.c
	gcc -g -Wall -c -o cd.o cd.c
	gcc -g -Wall -c -o etoile.o etoile.c
	gcc -g -Wall -c -o redirection.o redirection.c
	gcc -g -Wall -c -o slash.o slash.c
	gcc  slash.o etoile.o cd.o pwd.o lib.o redirection.o -o slash -lreadline

clean :
	rm -rf *~

cleanall :
	rm -rf *~ *.o
