OUTPUT_DIR := out

all: client serverC serverCS serverEE serverM
	
client: client.c utils/** networking/**
	gcc -g -Wall -Iutils -Inetworking -o $(OUTPUT_DIR)/client client.c utils/**.c networking/udp_server.c
	
serverC: serverC.c utils/** networking/**
	gcc -g -Wall -Iutils -Inetworking -o $(OUTPUT_DIR)/serverC serverC.c utils/**.c networking/udp_server.c data/credentials.c

serverCS: serverCS.c utils/** networking/** data/**
	gcc -g -Wall -Idata -Iutils -Inetworking -o $(OUTPUT_DIR)/serverCS serverCS.c serverSub.c utils/**.c networking/udp_server.c data/courses.c

serverEE: serverEE.c utils/** networking/** data/**
	gcc -g -Wall -Idata -Iutils -Inetworking -o $(OUTPUT_DIR)/serverEE serverEE.c serverSub.c utils/**.c networking/udp_server.c data/courses.c

serverM: serverM.c utils/** networking/**
	gcc -g -Wall -Idata -Iutils -Inetworking -o $(OUTPUT_DIR)/serverM serverM.c

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

run:
	./serverC && ./serverCS && ./serverEE && ./serverM && ./client

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM