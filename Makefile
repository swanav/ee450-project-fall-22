# build an executable named myprog from myprog.c

all: client serverC serverCS serverEE serverM
	
client: client.c utils/** networking/**
	gcc -g -Wall -o client client.c
	
serverC: serverC.c utils/** networking/**
	gcc -g -Wall -Iutils -Inetworking -o serverC serverC.c utils/**.c networking/udp_server.c

serverCS: serverCS.c utils/** networking/** data/**
	gcc -g -Wall -Idata -Iutils -Inetworking -o serverCS serverCS.c serverSub.c utils/**.c networking/udp_server.c data/courses.c

serverEE: serverEE.c utils/** networking/** data/**
	gcc -g -Wall -Idata -Iutils -Inetworking -o serverEE serverEE.c serverSub.c utils/**.c networking/udp_server.c data/courses.c

serverM: serverM.c utils/** networking/**
	gcc -g -Wall -o serverM serverM.c

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

run:
	./serverC && ./serverCS && ./serverEE && ./serverM && ./client

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM