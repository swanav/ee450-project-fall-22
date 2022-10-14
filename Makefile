# build an executable named myprog from myprog.c

all: client serverC serverCS serverEE serverM
	
client: client.c utils.c utils.h log.h
	gcc -g -Wall -o client client.c
	
serverC: serverC.c utils.c utils.h log.h
	gcc -g -Wall -o serverC serverC.c utils.c

serverCS: serverCS.c utils.c utils.h log.h
	gcc -g -Wall -o serverCS serverCS.c utils.c

serverEE: serverEE.c utils.c utils.h log.h
	gcc -g -Wall -o serverEE serverEE.c utils.c

serverM: serverM.c utils.c utils.h log.h
	gcc -g -Wall -o serverM serverM.c

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

run:
	./serverC && ./serverCS && ./serverEE && ./serverM && ./client

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM