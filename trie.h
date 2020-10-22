#ifndef TRIE_H
#define TRIE_H
#include <stdbool.h>
struct Line_list{
	int line_number;
	struct Line_list *next;
};

struct P_List{
	int doc_id;//id tou keimenou pou emfanizetai i leksi
	int path_id;//id tou path pou emfanizetai i leksi
	int count_per_id; //poses fores emfanizetai se ka8e keimeno
	char* f_name; //onoma arxeiou
	struct Line_list *line_list;
	struct P_List *next;
};

struct TrieNode{
	char key;
	struct TrieNode *next; //deixnei sto idio level,sta adelfia
	struct TrieNode *child; //deixnei sto epomeno level,sta paidia
	struct P_List *posting_list;
	bool isendofword;
	//int visited;
};
struct TrieNode* trieCreate();
void insert(struct TrieNode*, char*,char*,int,int,int);
struct P_List* search(struct TrieNode*, char*);
struct P_List* maxcount(struct TrieNode*, char*);
struct P_List* mincount(struct TrieNode*, char*);
void LineListDestroy(struct Line_list*);
void PostingListDestroy(struct P_List*);
void trieDestroy(struct TrieNode*);
#endif
