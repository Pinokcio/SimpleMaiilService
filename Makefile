project:	sendmail project.o parson.o
	gcc -o project project.o parson.o -pthread
sendmail:	sendmail.o common.o reentrant.o
	gcc -g -Wall -pthread -o sendmail sendmail.o common.o reentrant.o -lssl -lcrypto
project.o: project.c
	gcc -c project.c
sendmail.o:	sendmail.c
	gcc -g -Wall -pthread -c sendmail.c
common.o: common.c
		gcc -g -Wall -pthread -c common.c
reentrant.o: reentrant.c
		gcc -g -Wall -pthread -c reentrant.c
parson.o: parson.c
		gcc -c parson.c
