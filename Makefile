OUTPUT_DIR := out

all: client serverM serverC serverCS serverEE

client: client.c out_dir
	gcc -g -Wall -DCLIENT \
		-o $(OUTPUT_DIR)/client \
			client.c database.c log.c protocol.c networking.c utils.c \
		-lpthread

serverM: serverM.c out_dir
	gcc -g -Wall -DSERVER_M \
		-o $(OUTPUT_DIR)/serverM  \
			serverM.c database.c log.c protocol.c networking.c utils.c \
		-lpthread

serverC: serverC.c out_dir
	gcc -g -Wall -DSERVER_C \
		-o $(OUTPUT_DIR)/serverC \
			serverC.c database.c fileio.c log.c protocol.c networking.c utils.c

serverCS: serverCS.c out_dir
	gcc -g -Wall -DSERVER_CS \
		-o $(OUTPUT_DIR)/serverCS \
			serverCS.c database.c department_server.c fileio.c log.c protocol.c networking.c utils.c

serverEE: serverEE.c out_dir
	gcc -g -Wall -DSERVER_EE \
		-o $(OUTPUT_DIR)/serverEE \
			serverEE.c database.c department_server.c fileio.c log.c protocol.c networking.c utils.c

out_dir:
	mkdir -p $(OUTPUT_DIR)

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile README.md
	gzip ee450_Swaroop_Swanav_1349754053.tar

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM out
