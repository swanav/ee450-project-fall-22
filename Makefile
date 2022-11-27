OUTPUT_DIR := out

all: client serverC serverCS serverEE serverM
	
client: client.c utils/**  out_dir
	gcc -g -Wall -Iutils \
		-o $(OUTPUT_DIR)/client \
			client.c protocol.c tcp.c \
			data/courses.c data/credentials.c \
			utils/log.c utils/utils.c \
		-lpthread

serverM: serverM.c utils/** data/** out_dir
	gcc -g -Wall -Idata -Iutils \
		-o $(OUTPUT_DIR)/serverM  \
			serverM.c protocol.c tcp.c udp.c \
			data/courses.c data/credentials.c \
			utils/log.c utils/utils.c \
		-lpthread

serverC: serverC.c utils/**  out_dir
	gcc -g -Wall -Iutils \
		-o $(OUTPUT_DIR)/serverC \
			serverC.c fileio.c protocol.c udp.c \
			data/courses.c data/credentials.c \
			utils/log.c utils/utils.c

serverCS: serverCS.c utils/** data/** out_dir
	gcc -g -Wall -Idata -Iutils \
		-o $(OUTPUT_DIR)/serverCS \
			serverCS.c fileio.c protocol.c serverSub.c udp.c \
			data/courses.c \
			utils/log.c utils/utils.c

serverEE: serverEE.c utils/** data/** out_dir
	gcc -g -Wall -Idata -Iutils \
		-o $(OUTPUT_DIR)/serverEE \
			serverEE.c fileio.c protocol.c serverSub.c udp.c \
			data/courses.c \
			utils/log.c utils/utils.c

out_dir:
	mkdir -p $(OUTPUT_DIR)

bundle:
	tar cvf ee450_Swaroop_Swanav_1349754053.tar *.[ch] Makefile
	gzip ee450_Swaroop_Swanav_1349754053.tar

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM out
