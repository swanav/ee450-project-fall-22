OUTPUT_DIR := out

all: client serverC serverCS serverEE serverM
	
client: client.c utils/** networking/** out_dir
	gcc -g -Wall -Iutils -Inetworking -o $(OUTPUT_DIR)/client client.c utils/**.c data/**.c networking/**.c protocol.c  -lpthread
	
serverC: serverC.c utils/** networking/** out_dir
	gcc -g -Wall -Iutils -Inetworking -o $(OUTPUT_DIR)/serverC serverC.c utils/**.c networking/**.c data/**.c protocol.c

serverCS: serverCS.c utils/** networking/** data/** out_dir
	gcc -g -Wall -Idata -Iutils -Inetworking -o $(OUTPUT_DIR)/serverCS serverCS.c serverSub.c protocol.c utils/**.c networking/**.c data/**.c

serverEE: serverEE.c utils/** networking/** data/** out_dir
	gcc -g -Wall -Idata -Iutils -Inetworking -o $(OUTPUT_DIR)/serverEE serverEE.c serverSub.c protocol.c utils/**.c networking/**.c data/**.c

serverM: serverM.c utils/** networking/** out_dir
	gcc -g -Wall -Idata -Iutils -Inetworking -o $(OUTPUT_DIR)/serverM serverM.c protocol.c utils/**.c networking/**.c data/**.c

out_dir:
	mkdir -p $(OUTPUT_DIR)

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

run:
	./serverC && ./serverCS && ./serverEE && ./serverM && ./client

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM out
