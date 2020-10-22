all: main.o trie.o
	g++ main.o trie.o -o jobExecutor
main.o: main.c
	g++ -c main.c
trie.o: trie.c trie.h
	g++ -c trie.c
clean:
	rm *.o jobExecutor
	rm -rf log
