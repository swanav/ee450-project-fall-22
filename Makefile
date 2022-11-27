OUTPUT_DIR := out

all: client serverC serverCS serverEE serverM
	
client: client.c out_dir
	gcc -g -Wall \
		-o $(OUTPUT_DIR)/client \
			client.c protocol.c tcp.c \
			data/courses.c \
			log.c utils.c \
		-lpthread

serverM: serverM.c data/** out_dir
	gcc -g -Wall -Idata \
		-o $(OUTPUT_DIR)/serverM  \
			serverM.c protocol.c tcp.c udp.c \
			data/courses.c credentials.c \
			log.c utils.c \
		-lpthread

serverC: serverC.c  out_dir
	gcc -g -Wall \
		-o $(OUTPUT_DIR)/serverC \
			serverC.c fileio.c protocol.c udp.c \
			data/courses.c credentials.c \
			log.c utils.c

serverCS: serverCS.c data/** out_dir
	gcc -g -Wall -Idata \
		-o $(OUTPUT_DIR)/serverCS \
			serverCS.c fileio.c protocol.c department_server.c udp.c \
			data/courses.c \
			log.c utils.c

serverEE: serverEE.c data/** out_dir
	gcc -g -Wall -Idata \
		-o $(OUTPUT_DIR)/serverEE \
			serverEE.c fileio.c protocol.c department_server.c udp.c \
			data/courses.c \
			log.c utils.c

out_dir:
	mkdir -p $(OUTPUT_DIR)

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM out
