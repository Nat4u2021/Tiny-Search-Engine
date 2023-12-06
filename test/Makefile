CFLAGS=-Wall -pedantic -std=c11 -I../utils -L../lib -g
LIBS=-lutils -lcurl

all:			pageio_test indexio_test lqueue_test lhash_test

pageio_test:
				gcc $(CFLAGS) pageio_test.c $(LIBS) -o $@

indexio_test:
				gcc $(CFLAGS) indexio_test.c $(LIBS) -o $@

lqueue_test:
				gcc $(CFLAGS) lqueue_test.c $(LIBS) -o $@

lhash_test:
				gcc $(CFLAGS) lhash_test.c $(LIBS) -o $@

clean: 
				rm -f *.o pageio_test indexio_test lqueue_test lhash_test
