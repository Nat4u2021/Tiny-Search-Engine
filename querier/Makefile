CFLAGS=-Wall -pedantic -std=c11 -I../utils -L../lib -g
LIBS=-lutils -lcurl

query:
				gcc $(CFLAGS) query.c $(LIBS) -o $@

clean: 
				rm -f *.o query
