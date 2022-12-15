BUNDLE_DIR := ee450_Swaroop_Swanav_swanavsw

SRC_DIR := src
OUT_DIR := out

all: client serverM serverC serverCS serverEE

client: $(SRC_DIR)/client.c
	gcc -g -Wall -DCLIENT \
		-o $(OUT_DIR)/client \
			$(SRC_DIR)/client.c \
			$(SRC_DIR)/database.c \
			$(SRC_DIR)/log.c \
			$(SRC_DIR)/protocol.c \
			$(SRC_DIR)/networking.c \
			$(SRC_DIR)/utils.c \
		-lpthread

serverM: $(SRC_DIR)/serverM.c
	gcc -g -Wall -DSERVER_M \
		-o $(OUT_DIR)/serverM \
			$(SRC_DIR)/serverM.c \
			$(SRC_DIR)/database.c \
			$(SRC_DIR)/log.c \
			$(SRC_DIR)/protocol.c \
			$(SRC_DIR)/networking.c \
			$(SRC_DIR)/utils.c \
		-lpthread

serverC: $(SRC_DIR)/serverC.c
	gcc -g -Wall -DSERVER_C \
		-o $(OUT_DIR)/serverC \
			$(SRC_DIR)/serverC.c \
			$(SRC_DIR)/database.c \
			$(SRC_DIR)/fileio.c \
			$(SRC_DIR)/log.c \
			$(SRC_DIR)/protocol.c \
			$(SRC_DIR)/networking.c \
			$(SRC_DIR)/utils.c

serverCS: $(SRC_DIR)/serverCS.c
	gcc -g -Wall -DSERVER_CS \
		-o $(OUT_DIR)/serverCS \
			$(SRC_DIR)/serverCS.c \
			$(SRC_DIR)/database.c \
			$(SRC_DIR)/department_server.c \
			$(SRC_DIR)/fileio.c \
			$(SRC_DIR)/log.c \
			$(SRC_DIR)/protocol.c \
			$(SRC_DIR)/networking.c \
			$(SRC_DIR)/utils.c

serverEE: $(SRC_DIR)/serverEE.c
	gcc -g -Wall -DSERVER_EE \
		-o $(OUT_DIR)/serverEE \
			$(SRC_DIR)/serverEE.c \
			$(SRC_DIR)/database.c \
			$(SRC_DIR)/department_server.c \
			$(SRC_DIR)/fileio.c \
			$(SRC_DIR)/log.c \
			$(SRC_DIR)/protocol.c \
			$(SRC_DIR)/networking.c \
			$(SRC_DIR)/utils.c

bundle:
	mkdir -p $(BUNDLE_DIR)
	cp $(SRC_DIR)/*.[ch] Makefile README.md $(BUNDLE_DIR)
	tar cvfz $(BUNDLE_DIR).tar $(BUNDLE_DIR)
	gzip $(BUNDLE_DIR).tar

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM $(OUT_DIR)
