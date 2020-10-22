#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "trie.h"

struct Line_list* Line_listCreate(){
  struct Line_list *lNode=NULL;

  lNode=(struct Line_list*)malloc(sizeof(struct Line_list));

  if(lNode){
    lNode->line_number=-1;
    lNode->next=NULL;
  }
  return lNode;
}
struct P_List* P_ListCreate() { //dimiourgia komvou posting list
    struct P_List *plNode = NULL;

    plNode = (struct P_List*)malloc(sizeof(struct P_List));

    if (plNode)
    {
      plNode->doc_id=-1;
      plNode->path_id =-1;
      plNode->line_list=NULL;
		  plNode->count_per_id=0;
      plNode->f_name=NULL;
      plNode->next=NULL;
    }
    return plNode;
}

struct TrieNode* trieCreate() { //dimiourgia rizas/komvou
    struct TrieNode *pNode = NULL;

    pNode = (struct TrieNode *)malloc(sizeof(struct TrieNode));

    if (pNode)
    {
      pNode->key = '\0';
		  pNode->child=NULL;
      pNode->next=NULL;
      pNode->posting_list=NULL;
		  pNode->isendofword=false;
    //  pNode->visited=0;
    }
    return pNode;
}

void insert(struct TrieNode* root, char* word,char* file_name,int document_id,int path_id,int line_number) {
    struct TrieNode *pCrawl = root;
    if (root){ //an to root den einai NULL
		int i=0;
    while( word[i] !='\0' && word[i]!='\n'){
     	if(pCrawl->child==NULL){ //an i riza/komvos den exei kanena paidi,dimiourgise ena
     		pCrawl->child=trieCreate();
     		pCrawl->child->key=word[i];
     	  pCrawl = pCrawl->child;

		 }
		 else if(pCrawl->child!=NULL){ //an o komvos pou vriskomaste exei paidi,diatrexoume to epipedo tou paidiou
			pCrawl=pCrawl->child;
		 	while(1){
		 		if(pCrawl->key==word[i]){ //an to gramma pros eisagwgi uparxei
			 		break;
			 	}
			 	else if(pCrawl->key!=word[i] && pCrawl->next!=NULL){//an den tairiazei to gramma kai uparxei epomenos(adelfikos) komvos
			 		pCrawl=pCrawl->next; //deikse se auton
				}
				else if(pCrawl->key!=word[i] && pCrawl->next==NULL){ //an den uparxei to gramma sto epipedo
					pCrawl->next=trieCreate();
					pCrawl->next->key=word[i];
					pCrawl=pCrawl->next;
					break;
				}

			 }
		 }
        i++;
    }
  if(pCrawl->isendofword!=true){ //an i leksi mpike gia proti fora sto trie
	   pCrawl->isendofword=true;
     pCrawl->posting_list=P_ListCreate(); //dimiourgia posting list
     pCrawl->posting_list->line_list=Line_listCreate(); //dimiourgia listas apo8ikeusis grammwn sto posting list
     pCrawl->posting_list->line_list->line_number=line_number;
     pCrawl->posting_list->doc_id=document_id;
     pCrawl->posting_list->path_id=path_id;
     pCrawl->posting_list->count_per_id=1;
     pCrawl->posting_list->f_name=(char *)malloc((strlen(file_name)+1));
     strcpy(pCrawl->posting_list->f_name,file_name);

  }
  else{ //an i leksi uparxei idi sto trie,enimerwsi posting list
    struct P_List* list_ptr=pCrawl->posting_list;
    while(1){
      if(list_ptr->doc_id==document_id && list_ptr->path_id==path_id){
        list_ptr->count_per_id++;
        while(list_ptr->line_list!=NULL){
          list_ptr->line_list=list_ptr->line_list->next;
        }
        list_ptr->line_list=Line_listCreate();
        list_ptr->line_list->line_number=line_number;
        break;
      }
      else if((list_ptr->doc_id!=document_id || list_ptr->path_id!=path_id) && list_ptr->next!=NULL){

        list_ptr=list_ptr->next; //pame sto telos tou posting list
      }
      else if((list_ptr->doc_id!=document_id || list_ptr->path_id!=path_id) && list_ptr->next==NULL){
        //an ftasame sto telos tou posting list kai dn vre8ike o sunduasmos path kai doc id
        list_ptr->next=P_ListCreate();
        list_ptr=list_ptr->next;
        list_ptr->doc_id=document_id;
        list_ptr->path_id=path_id;
        list_ptr->f_name=(char *)malloc((strlen(file_name)+1));
        strcpy(list_ptr->f_name,file_name);
        list_ptr->line_list=Line_listCreate();
        list_ptr->line_list->line_number=line_number;
        list_ptr->count_per_id++;
        break;
      }
    }

  }
}
}

struct P_List* search(struct TrieNode* root, char* word){
	struct TrieNode *ptr = root;
	int length=strlen(word);
	int i=0;
	for(i=0;i<length;i++){ //length-1 gia to '\0'
		if(ptr->child!=NULL){
			ptr=ptr->child;//pointer deixnei sto paidi tou
		}
		else{
	//		printf("den uparxei paidi\n");
			return NULL;
		}
		while(1){
			if(ptr->key==word[i]){ //vre8ike o xaraktiras,pigaine ston epomeno xaraktira tis leksis
				break;
			}
			else if(ptr->key!=word[i] && ptr->next!=NULL){//an den vre8ike alla uparxoun adelfia,psakse ta adelfia
				ptr=ptr->next;
			}
			else if(ptr->key!=word[i] && ptr->next==NULL){
		//		printf("den vre8ike kapoios xaraktiras,ara den uparxei i leksi\n");
				return NULL;
			}
		}
	}
	//an vgei apo tin for xwris na kanei return,eimaste se antistoixia tis leksis me ena path tou trie
	//chekaroume na prokeitai gia leksi i apla gia meros mias allis leksis
    if(ptr->isendofword==true){
		    return ptr->posting_list;
	     }
	  else{
          return NULL;
	      }

}
struct P_List* maxcount(struct TrieNode* root, char* word){
  struct P_List* p_list=search(root,word); //vriskoume to posting list tis leksis
  struct P_List* return_list;
  if(p_list){ //an uparxei posting list
    return_list=p_list;
    while(p_list->next!=NULL){ //testaroume oles tis eggrafes tis posting list
      if(p_list->next->count_per_id>return_list->count_per_id){//vriskoume to max count per id
        return_list=p_list->next;
      }
      p_list=p_list->next;
    }
    return return_list;
  }
  else{
    return NULL; //an den vre8ike posting_list,den uparxei i leksi ara gurname NULL
  }

}

struct P_List* mincount(struct TrieNode* root, char* word){
  struct P_List* p_list=search(root,word); //vriskoume to posting list tis leksis
  struct P_List* return_list;
  if(p_list){ //an uparxei posting list
    return_list=p_list;
    while(p_list->next!=NULL){ //testaroume oles tis eggrafes tis posting list
      if(p_list->next->count_per_id<return_list->count_per_id && p_list->next->count_per_id>0){//vriskoume to min count per id
        return_list=p_list->next;
      }
      p_list=p_list->next;
    }
    return return_list;
  }
  else{
    return NULL; //an den vre8ike posting_list,den uparxei i leksi ara gurname NULL
  }

}
void LineListDestroy(struct Line_list* llist){
    if(llist->next!=NULL)
      LineListDestroy(llist->next);
    free(llist);
}
void PostingListDestroy(struct P_List* list){
    if(list->next!=NULL)
      PostingListDestroy(list->next);
    if(list->line_list!=NULL)
      LineListDestroy(list->line_list);

    free(list->f_name);
    free(list);
}

void trieDestroy(struct TrieNode* root){
  if(root->child!=NULL)
    trieDestroy(root->child);

  if(root->next!=NULL)
    trieDestroy(root->next);

  if(root->posting_list!=NULL)
    PostingListDestroy(root->posting_list);
  free(root);
}
