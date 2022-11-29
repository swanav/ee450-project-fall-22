BUNDLE_DIR := ee450_Swaroop_Swanav_swanavsw

all: client serverM serverC serverCS serverEE

client: client.c
	gcc -g -Wall -DCLIENT \
		-o client client.c database.c log.c protocol.c networking.c utils.c \
		-lpthread

serverM: serverM.c
	gcc -g -Wall -DSERVER_M \
		-o serverM serverM.c database.c log.c protocol.c networking.c utils.c \
		-lpthread

serverC: serverC.c
	gcc -g -Wall -DSERVER_C \
		-o serverC serverC.c database.c fileio.c log.c protocol.c networking.c utils.c

serverCS: serverCS.c
	gcc -g -Wall -DSERVER_CS \
		-o serverCS \
			serverCS.c database.c department_server.c fileio.c log.c protocol.c networking.c utils.c

serverEE: serverEE.c
	gcc -g -Wall -DSERVER_EE \
		-o serverEE serverEE.c database.c department_server.c fileio.c log.c protocol.c networking.c utils.c

bundle:
	mkdir -p $(BUNDLE_DIR)
	cp *.[ch] Makefile README.md $(BUNDLE_DIR)
	tar cvfz $(BUNDLE_DIR).tar $(BUNDLE_DIR)
	gzip $(BUNDLE_DIR).tar

clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM out
