all: indexer.o sorted-list.o tokenizer.o
	gcc -o index indexer.o sorted-list.o tokenizer.o

indexer.o: indexer.c indexer.h uthash.h
	gcc -c indexer.c

sorted-list.o: sorted-list.c sorted-list.h
	gcc -c sorted-list.c

tokenizer.o: tokenizer.c tokenizer.h
	gcc -c tokenizer.c

clean:
	rm -f index
	rm -f *.o