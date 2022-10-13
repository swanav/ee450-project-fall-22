# build an executable named myprog from myprog.c

all: client serverC serverCS serverEE serverM
	
client:
	gcc -g -Wall -o client client.c
	
serverC:
	gcc -g -Wall -o serverC serverC.c

serverCS:
	gcc -g -Wall -o serverCS serverCS.c

serverEE:
	gcc -g -Wall -o serverEE serverEE.c

serverM:
	gcc -g -Wall -o serverM serverM.c


clean:
	$(RM) -r client serverEE serverCS serverC serverM *.dSYM