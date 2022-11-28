OUTPUT_DIR := out

all: client serverC serverCS serverEE serverM

client: client.c out_dir
	gcc -g -Wall \
		-o $(OUTPUT_DIR)/client \
			client.c courses.c log.c protocol.c tcp.c utils.c \
		-lpthread

serverM: serverM.c out_dir
	gcc -g -Wall -Idata \
		-o $(OUTPUT_DIR)/serverM  \
			serverM.c courses.c credentials.c log.c protocol.c tcp.c udp.c utils.c \
		-lpthread

serverC: serverC.c  out_dir
	gcc -g -Wall \
		-o $(OUTPUT_DIR)/serverC \
			serverC.c credentials.c fileio.c log.c protocol.c udp.c utils.c

serverCS: serverCS.c out_dir
	gcc -g -Wall -Idata \
		-o $(OUTPUT_DIR)/serverCS \
			serverCS.c courses.c department_server.c fileio.c log.c protocol.c udp.c utils.c

serverEE: serverEE.c out_dir
	gcc -g -Wall -Idata \
		-o $(OUTPUT_DIR)/serverEE \
			serverEE.c courses.c department_server.c fileio.c log.c protocol.c udp.c utils.c

out_dir:
	mkdir -p $(OUTPUT_DIR)

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM out
