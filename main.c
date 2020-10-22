#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/poll.h>
#include <time.h>
#include "trie.h"
#define PERMS 0666

void signal_handler1(int signum){
}
int main(int argc,char* argv[]){
  char* filename;
  char* line=NULL;
  char c=0;
  char* pch=NULL;
  char* point=NULL;
  char *command_buffer = NULL;
  char search_error[36]="Word wasn't found in any documents\n";
  char search_buffer[10000];
  struct P_List* posting_list=NULL;
  int W=0,i=0,j=0,num_of_paths=0,counter=0,n=0,size=0,array_size=0,k=0,lines=0,v1=0,v2=0,v3=0,doc_count=0,line_number=0,read_command;
  int flag=0,count=0,max_count=0,min_count=0,total_max_count=0,total_min_count=0,max_temp=0,min_temp=0;
  int wc_bytes=0,wc_words=0,wc_lines=0,wc_temp=0,deadline=0,found=0;
  size_t len=0;
  size_t total_len=0,max_len=0;
  clock_t begin,end;
  double time_spent=0.0,dead_line=0.0;
  DIR* directory;
  struct dirent* dir;
  char* pos=NULL;
  if(argc==3){ //an do8ikan 2 parametroi,autoi 8a einai anagkastika -d kai onoma arxeiou
  	if(strcmp(argv[1],"-d")==0){
  		filename=(char *)malloc(sizeof(strlen(argv[1])+1));
  		filename=argv[2];
  		W=5; //default ari8mos gia workers
	  }
	  else{
		  printf("wrong parameters given\n");
		  return -1;
	  }
 	}
 	else if(argc==5){//an do8ikan 4 parametroi,exoume 2 epiloges gia tin seira tous
		if(strcmp(argv[1],"-d")==0 && strcmp(argv[3],"-w")==0){
			filename=(char *)malloc(sizeof(strlen(argv[1])+1));
  		filename=argv[2];
  		W=atoi(argv[4]);
		}
		else if(strcmp(argv[1],"-w")==0 && strcmp(argv[3],"-d")==0){
			filename=(char *)malloc(sizeof(strlen(argv[4])+1));
  		filename=argv[4];
  		W=atoi(argv[2]);
		}
		else{
			printf("wrong parameters given\n");
			return -1;
		}
	}
	else{
		printf("wrong parameters given\n");
		return -1;
	}
  if(W<1){
    printf("Number of workers should be positive\n");
    return -1;
  }
  printf("number of workers=%d\n",W);
 //diavasma docfile
  FILE* file=fopen(filename,"r"); //anoigma arxeiou
  if(file == NULL) {
    printf("Error in opening file\n");
    return(-1);
  }
  while(c!=EOF){ //vriskoume posa paths exei to docfile
    c = fgetc(file);
    if(c == '\n')
      num_of_paths++;
  }
  fclose(file);
  if(W>num_of_paths){//an W pio polloi apo paths,tote to W ginetai num_of_paths
    W=num_of_paths;
    printf("resetting number of workers\n");
  }

  //printf("num_of_paths=%d\n",num_of_paths);
  int chars_per_path[num_of_paths];
  for(i=0;i<num_of_paths;i++){
     chars_per_path[i]=0;
  }
  file=fopen(filename,"r");
  if(file == NULL) {
    printf("Error in opening file\n");
    return(-1);
  }
  i=0;
  for(;;){
    c = fgetc(file);
	   if(c!='\n' && c!=EOF){
       chars_per_path[i]++;
     }
     else if( c=='\n'){
       i++;
	   }
	   else if(c==EOF){
	      break;
	   }
  }

  fclose(file);
  file=fopen(filename,"r");
  if(file == NULL) {
    printf("Error in opening file\n");
    return(-1);
  }

  char **array_of_paths=(char **)malloc(num_of_paths* sizeof (char* ));//pinakas apo8ikeusis twn paths
  for(i=0;i<num_of_paths;i++){
    array_of_paths[i]=(char *)malloc(chars_per_path[i] * sizeof(char)+2); //desmeuoume dunamika xwro gia to ka8e path
  }
  i=0;


  while(!feof(file)){
  	fgets(array_of_paths[i],chars_per_path[i] +2,file); //vazoume ta keimena ston pinaka array_of_docs
  	i++;
  }
  fclose(file);
  for(i=0;i<num_of_paths;i++){
    printf("%s\n",array_of_paths[i]);//ektupwsi twn paths
  }

  pid_t childpid;
  pid_t mypid=getpid();
  signal(SIGUSR1,signal_handler1);

  int read_fd[W];
  int write_fd[W];
  for(i=0;i<W;i++){
    read_fd[i]=0;
    write_fd[i]=0;
  }
  const char* myfifo="/tmp/myfifo";
  char**fifo_write=(char**)malloc(W* sizeof(char*));
  char**fifo_read=(char**)malloc(W* sizeof(char*));
  for(i=0;i<W;i++){
    fifo_write[i]=(char *)malloc(strlen(myfifo) * sizeof(char));
    snprintf(fifo_write[i],strlen(myfifo)+3,"%s %d",myfifo,i);//ftiaxnoume ta onomata twn pipes
  //  printf("names=%s\n",fifo_write[i]);

    fifo_read[i]=(char *)malloc(strlen(myfifo) * sizeof(char));
    snprintf(fifo_read[i],strlen(myfifo)+4,"%s %d",myfifo,i+W);//gia read/write antistoixa
  //  printf("names=%s\n",fifo_read[i]);
  }
  for(int i=0;i<W;i++){
    if ( (mkfifo(fifo_write[i], PERMS) < 0) && (errno !=EEXIST) ) {//dimiourgoume ta W named pipes gia writing
      perror("can't create fifo");
    }
    if ( (mkfifo(fifo_read[i], PERMS) < 0) && (errno !=EEXIST) ) {//dimiourgoume ta W named pipes gia reading
      perror("can't create fifo");
    }
  }

  int**workers_id=(int**)malloc(W* sizeof(int*));
  for(int i=0;i<W;i++){
    workers_id[i]=(int*)malloc(sizeof(int));
  }
  for(int i=0;i<W;i++){ // loop will run W times
      *workers_id[i]=fork(); //kratame se pinaka mege8ous W ta id twn child processes
      if (*workers_id[i]== -1){
        perror("Failed to fork");
        return 1;
      }
      if(*workers_id[i]==0){ //gia ka8e worker
        char buff[1000];
        char full_path[100];
        char temp_buff[100];
        if ((write_fd[i] = open(fifo_write[i], O_RDONLY | O_NONBLOCK )) < 0) {//anoigma fifos gia diavasma apo executor
          perror("worker can't open read_fd");
        }
        printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
        pause();
      //  printf("paused!\n");
        char log_filename[20];
        snprintf(log_filename,20,"log/worker%d",getpid());
        time_t t = time(NULL);
        FILE* log_file=fopen(log_filename,"ab+");//anoigma log_file
        if(!log_file)
          perror("fopen");

        j=0;
        size=0;
        array_size=num_of_paths/W; //posa paths 8a exei na dieu8unei o worker
        if(num_of_paths%W!=0)
          array_size++;
        char **paths=(char **)malloc(array_size* sizeof (char* )); //pinakas pou krataei ta paths pou 8a diavasei ka8e child process
        if ((n= read(write_fd[i], buff,1000)) <= 0){
          perror("worker: filename read error ");
        }
        buff[n] ='\0'; // null terminate filename
        n=0;
        while(buff[n]!='\0'){ //orismos katallilou mege8ous gia apo8ikeusi ka8e path
          size++;
          if(buff[n]=='\n'){
            paths[j]=(char *)malloc(size * sizeof(char));
          //  paths[j]=NULL;
            j++;
            size=0;
          }
          n++;
        }
        j=0;
        n=0;
        k=0;
        int last_end=0;
        char ch;
        while(buff[n]!='\0'){        //pername ta paths tou ka8e child ston pinaka paths
          k++;
          if(buff[n]=='\n'){
            strncpy(paths[j],&buff[last_end],k);
            last_end+=k;
            k=0;
            j++;
          }
        	n++;
        }
        //array_size dilwnei posa paths exoume
        char temp[100];
        for(j=0;j<array_size;j++){
          if((pos=strchr(paths[j],'\n'))!=NULL) //vgazoume to \n apo to pathname
	         *pos='\0';
        }
        doc_count=0;
        struct TrieNode* node = trieCreate();
        for(j=0;j<array_size;j++){
          directory=opendir(paths[j]);
          if(!directory){
            perror("error in opening dir");
          }
          else{
            while((dir=readdir(directory)) != NULL){ //oso diavazoume ta paths
              if(dir->d_name[0]!='.' && dir->d_name[0]!='\0'){
                doc_count++; //posa documents exei to path auto
              }
            }
            closedir(directory);
          }
        }
        char*** document_array=(char***)malloc(doc_count * sizeof(char**)); //3d array gia ta document ka8e worker
      v2=0;
      j=0;
      v1=0;
      while(paths[j]){
          directory=opendir(paths[j]);
          if(!directory){
            //perror("error in opening dir");
          }
          else{
            while((dir=readdir(directory)) != NULL){ //oso diavazoume ta paths
              if(dir->d_name[0]!='.' && dir->d_name[0]!='\0'){
                for(k=0;k<100;k++)
                  temp[k]='\0';
                strcpy(temp,paths[j]);
                strcat(temp,"/");
                strcat(temp,dir->d_name); //sto temp to onoma tou arxeiou
              //  printf("temp=--------------%s\n",temp);
                file=fopen(temp,"r");
                if(file == NULL) {
                  printf("Error in opening file\n");
                  return(-1);
                }
                lines=0;
                ch='0';
                while(ch!=EOF){ //vriskoume posa keimena exei to docfile
                  ch = fgetc(file);
                  if(ch == '\n')
                  {
                    lines++;
                    wc_lines++;//kratame sunoliko pli8os grammwn(gia entoli wc)
                  }
                }
                if(lines==0){//an anoi3ei to arxeio alla den vrei \n,tote exei 1 mono grammi
                  lines=1;}
                fclose(file);
                file=fopen(temp,"r");
                document_array[v1]=(char**)malloc(lines * sizeof(char*));//v1 doc_id
                for(v2=0;v2<lines;v2++){  //arxikopoioume ta bytes pou kaname malloc me NULL
                  document_array[v1][v2]=NULL;
                }
                v2=0;
                for(v2=0;v2<lines;v2++){
                  n=getline(&document_array[v1][v2],&len,file);//apo8ikeusi keimenwn
                  if(n>max_len)
                    max_len=n;
                    wc_bytes+=n;
              //    printf("getline evale %s me %d chars\n",document_array[v1][v2],n);
                  pch = strtok (document_array[v1][v2]," ,.-");
                  while (pch != NULL)
                  {
      	             if(pch!=NULL){
      	  	           // printf ("INSERTING %s from %s\n",pch,dir->d_name);
                        wc_words++;
      	  	            insert(node,pch,dir->d_name,v1,j,v2); //v1=document_id,j=path_id,v2=line_number
      	               }
                      pch = strtok (NULL," ");
                    }
                }
                fclose(file);
                file=fopen(temp,"r");
                v3=0;
                while(!feof(file)){
                  fgets(document_array[v1][v3],max_len+1,file); //vazoume ta keimena ston pinaka array_of_docs
                  v3++;
                }
                v1++;
                fclose(file);
              }
            }
            closedir(directory);
          }
          j++;
        }
        for(j=0;j<array_size;j++){
        //  if(paths[j]!=NULL)
          //  printf("paths=%s\n",paths[j]);//ektupwsi paths
        }
        if ((read_fd[i] = open(fifo_read[i], O_WRONLY | O_NONBLOCK)) < 0) {//anoigma pipe pou 8a grafei o worker ta result tou
          perror("worker can't open read_fd");
        }
      while(1){
      //  printf("worker waiting a query!\n");
        pause();
        printf("worker working on the query!\n");
        struct tm tm = *localtime(&t);//vazoume sto log file to date pou ir8e to query
        fflush(log_file);
        if ((n= read(write_fd[i], buff,1000)) <= 0){
          perror("worker: read error ");
        }
        buff[n] ='\0'; // null terminate filename
      //  printf("message was:%s\n",buff);
        pch = strtok (buff," ,.-");
        if(strcmp(pch,"/search")==0){ //analoga me tin 1i leksi tou query,kaloume to analogo command
          flag=1;
        }
        else if(strcmp(pch,"/maxcount")==0){
          flag=2;
          fprintf(log_file,"%d-%d-%d %d %d %d:", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
          fprintf(log_file,"maxcount:");
        }
        else if(strcmp(pch,"/mincount")==0){
          flag=3;
          fprintf(log_file,"%d-%d-%d %d %d %d:", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
          fprintf(log_file,"mincount:");
        }
        else if(strcmp(pch,"/wc\n")==0){
          fprintf(log_file,"%d-%d-%d %d %d %d:", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
          fprintf(log_file,"wc");
          fprintf(log_file,"\n");
          flag=4;
        }
        else if(strcmp(pch,"/exit\n")==0){
          flag=5;
          printf("Exiting program!\n");//apodesmeusi mnimis!!!!!!
        }
        if(flag==1){//search
          found=0;
          pch=strtok(NULL," ,.-");
          while(strcmp(pch,"-d")!=0){//gia ka8e leksi mexri to -d
            printf("pch=%s\n",pch);
            posting_list=search(node,pch);
            fprintf(log_file,"%d-%d-%d %d %d %d:", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            fprintf(log_file,"search:");
            fprintf(log_file,"%s",pch);//enimerwsi log_file
             while(posting_list!=NULL){
               strcpy(full_path,paths[posting_list->path_id]);
               strcat(full_path,"/");
               strcat(full_path,posting_list->f_name); //sto temp to onoma tou arxeiou
               strcat(full_path,"\n");
               if ((write(read_fd[i],full_path,strlen(full_path))) ==-1){//grafoume to full path pou vre8ike i leksi
                 perror("write");
               }
               if((pos=strchr(full_path,'\n'))!=NULL) //vgazoume to \n apo to path
               *pos='\0';
               fprintf(log_file,":%s",full_path);//pros8iki path sto log
               fflush(log_file);
               while(posting_list->line_list!=NULL){//gia oles tis grammes pou vre8ike i leksi sto keimeno
              //   printf("line_number=%d\n",posting_list->line_list->line_number);
                 snprintf(search_buffer,sizeof(search_buffer),"line_number=%d,line_content=%s",posting_list->line_list->line_number,document_array[posting_list->doc_id][posting_list->line_list->line_number]);
                 write(read_fd[i],search_buffer,strlen(search_buffer));
                 posting_list->line_list=posting_list->line_list->next;
               }
               posting_list=posting_list->next;
             }
             fprintf(log_file,"\n");
             pch=strtok(NULL," ");
           }

        }
        else if(flag==2){//MAXCOUNT
          pch=strtok(NULL," ,.-");
      //    printf("pch=%s\n",pch);
          pch[strlen(pch)-1]='\0';
          posting_list=maxcount(node,pch);
          fprintf(log_file,"%s:",pch);//pros8iki string sto log
           if(posting_list!=NULL){
      //       printf("doc_id=%d,path_id=%d,file_name=%s\n",posting_list->count_per_id,posting_list->path_id,posting_list->f_name);
             strcpy(full_path,paths[posting_list->path_id]);
             strcat(full_path,"/");
             strcat(full_path,posting_list->f_name); //sto temp to onoma tou arxeiou
             snprintf(temp_buff,strlen(full_path)+20,"count=%d,path=%s",posting_list->count_per_id,full_path);
             fprintf(log_file,"%s",full_path);//pros8iki path sto log
             max_count=posting_list->count_per_id;
             if ((write(read_fd[i],&max_count,sizeof(max_count))) ==-1){//grafoume maxcount sto pipe
               perror("write");
             }
             if ((write(read_fd[i],temp_buff,strlen(temp_buff))) ==-1){//grafoume path sto pipe
               perror("write");
             }
           }
           else{//an den vre8ike i leksi
             max_count=0;
             printf("word wasn't found in any document!\n");
             if ((write(read_fd[i],&max_count,sizeof(max_count))) ==-1){
                 perror("write");
               }
           }
           fprintf(log_file,"\n");
        }
        else if(flag==3){//MINCOUNT
          pch=strtok(NULL," ,.-");
      //    printf("pch=%s\n",pch);
          pch[strlen(pch)-1]='\0';
          posting_list=mincount(node,pch);
          fprintf(log_file,"%s:",pch);//pros8iki string sto log
           if(posting_list!=NULL){
          //   printf("doc_id=%d,path_id=%d,file_name=%s\n",posting_list->count_per_id,posting_list->path_id,posting_list->f_name);
             strcpy(full_path,paths[posting_list->path_id]);
             strcat(full_path,"/");
             strcat(full_path,posting_list->f_name); //sto temp to onoma tou arxeiou
             snprintf(temp_buff,strlen(full_path)+20,"count=%d,path=%s",posting_list->count_per_id,full_path);
             fprintf(log_file,"%s",full_path);//pros8iki path sto log
             min_count=posting_list->count_per_id;
             if ((write(read_fd[i],&min_count,sizeof(min_count))) ==-1){//mincount sto pipe
               perror("write");
             }
             if ((write(read_fd[i],temp_buff,strlen(temp_buff))) ==-1){//path sto pipe
               perror("write");
             }
           }
           else{//an den vre8ike i leksi
             min_count=0;
             printf("word wasn't found in any document!\n");
             if ((write(read_fd[i],&min_count,sizeof(min_count))) ==-1){
                 perror("write");
               }
           }
           fprintf(log_file,"\n");
        }
        else if(flag==4){//WC
          if ((write(read_fd[i],&wc_bytes,sizeof(wc_bytes))) ==-1){
            perror("write");
          }
          if ((write(read_fd[i],&wc_words,sizeof(wc_words))) ==-1){
            perror("write");
          }
          if ((write(read_fd[i],&wc_lines,sizeof(wc_lines))) ==-1){
            perror("write");
          }
        }
        else if(flag==5){//exit
          trieDestroy(node); //apodesmeusi trie
          for(j=0;j<array_size;j++){//apodesmeusi pinaka pou kratage ta paths
            free(paths[j]);
          }
          free(paths);

          break;
        }
    }
        close(read_fd[i]);  //kleisimo twn pipes pou analogoun ston worker
        close(write_fd[i]);
        fclose(log_file); //kleisimo log file
        exit(1);
      }
    }
    if(mypid!=0){//parent
      //printf("executor\n");
      struct stat st = {0};
      if (stat("log", &st) == -1) {//dimiourgia dir gia ta log files
        if(mkdir("log",0777)!= -1){
        //  printf("dir created\n");
        }
        else
          perror("mkdir:");
      }
      sleep(2);
      struct pollfd fds[W];
      char output_buffer[100];
	    int ret=0;

      for(i=0;i<W;i++){
       if ((write_fd[i] = open(fifo_write[i], O_WRONLY )) < 0) {//anoigma write fifo
         perror("parent can't open write_fd");
       }
       if ((read_fd[i] = open(fifo_read[i], O_RDONLY | O_NONBLOCK)) < 0) { //anoigma read fifo
           perror("parent can't open read_fd");
         }
      // printf("write fifo opened.\n");
     }
     i=0;
     counter=0;
     while(counter<num_of_paths){//perasma se ka8e worker ta paths pou tou antistoixoun
       if((write(write_fd[i], array_of_paths[counter] ,strlen(array_of_paths[counter]))) ==  -1 && errno != EAGAIN){
         perror("worker write");
         exit(4);
       }
       i++;
       counter++;
       if(i==W)
        i=0;
    }

      for(i=0;i<W;i++){ //kleisimo pipes kai sima stous workers na sunexisoun
        printf("parent sending signal\n");
        kill(*workers_id[i],SIGUSR1);
      }
      for(i=0;i<W;i++){
        fds[i].fd=read_fd[i];
        fds[i].events=POLLIN;
      }
      sleep(2);//settaroun oi workers
while(1){
     wc_bytes=0;
     wc_words=0;
     wc_lines=0;
     total_max_count=0;
     total_min_count=999;
      sleep(1);
      printf("Give a command\n");
      len=0;
      read_command = getline(&command_buffer, &len, stdin);//diavasma query

     for(i=0;i<W;i++){

       if((write(write_fd[i],command_buffer,strlen(command_buffer))+1) == -1){//stelnoume to query stous workers
         perror("executor write");
         exit(4);
       }
     }
     pch = strtok (command_buffer," ,.-");
     if(strcmp(pch,"/search")==0){ //analoga me tin 1i leksi tou query,kaloume to analogo command
       flag=1;
       begin=clock();//arxizoume na metrame ton xrono
       printf("CALLING /SEARCH\n");
         pch=strtok(NULL," ");
         while(strcmp(pch,"-d")!=0){
           pch=strtok(NULL," ");
         }
           pch=strtok(NULL," ");
           deadline=atoi(pch);
           dead_line=(double)deadline; //kratame to deadline
     }
     else if(strcmp(pch,"/maxcount")==0){
       flag=2;
       printf("calling maxcount!\n");
     }
     else if(strcmp(pch,"/mincount")==0){
       flag=3;
       printf("calling mincount!\n");
     }
     else if(strcmp(pch,"/wc\n")==0){
       flag=4;
       printf("calling /wc!\n");
     }
     else if(strcmp(pch,"/exit\n")==0){
       flag=5;
       printf("Exiting program!\n");//apodesmeusi mnimis!!!!!!
     }
     else{
       printf("this is not a command,try again.\n");
       flag=6;
     }


     for(i=0;i<W;i++){ //sima stous workers na epeksergastoun to query
       printf("parent sending signal\n");
       kill(*workers_id[i],SIGUSR1);
     }
     sleep(1);
    if(flag==1){//search
      ret=poll(fds,W,-1);
     for(i=0;i<W;i++){ //diavasma oti egrapsan oi workers
       if(ret==-1)
        perror("poll");
       if(fds[i].revents & POLLIN){
         time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
        // printf("time spent=%f\n",time_spent);
        // printf("pipe is readable\n");
         n=read(read_fd[i],search_buffer,10000);
         if(time_spent<dead_line){//an o worker prolave na apantisei on time
           printf("search_result:::%s",search_buffer);//ektupwsi apotelesmatos
         }
         else
          printf("worker did not answer on time!\n");
        }
        else
          printf("---%d\n",fds[i].revents);
     }
    }
    else if(flag==2){//maxcount
      ret=poll(fds,W,-1);
     for(i=0;i<W;i++){ //diavasma oti egrapsan oi workers
       if(ret==-1)
        perror("poll");
       if(fds[i].revents & POLLIN){
 		   //  printf("pipe is readable\n");
         n=read(read_fd[i],&max_temp,sizeof(int));
         n=read(read_fd[i],command_buffer,100);
         if(max_temp>total_max_count){//kratame to oliko max
          total_max_count=max_temp;
          strcpy(output_buffer,command_buffer);
         }
         else if(max_temp==total_max_count){//se periptwsi isova8mias
           if(strcmp(command_buffer,output_buffer) < 0){//kratame to alfavitika mikrotero full path
             strcpy(output_buffer,command_buffer);
           }
         }
        }
        else
          printf("---%d\n",fds[i].revents);
     }
     if(total_max_count==0)
      printf("max_count result:::word wasnt found!!!\n");
      else{
        printf("max_count result:::%s\n",output_buffer);
      }
    }
    else if(flag==3){ //mincount
      ret=poll(fds,W,-1);
     for(i=0;i<W;i++){ //diavasma oti egrapsan oi workers
       if(ret==-1)
        perror("poll");
       if(fds[i].revents & POLLIN){
 		  //   printf("pipe is readable\n");
         n=read(read_fd[i],&min_temp,sizeof(int));
         n=read(read_fd[i],command_buffer,100);
         if(min_temp<total_min_count && min_temp>0){//kratame to oliko min
          total_min_count=min_temp;
          strcpy(output_buffer,command_buffer);
         }
         else if(min_temp==total_min_count){//se periptwsi isova8mias
           if(strcmp(command_buffer,output_buffer) < 0){//kratame to alfavitika mikrotero full path
             strcpy(output_buffer,command_buffer);
           }
         }
        }
     }
     if(total_min_count==999)
      printf("min_count result:::word wasnt found!!!\n");
      else{
        printf("min_count result:::%s\n",output_buffer);
      }
    }
    else if(flag==4){//wc
      ret=poll(fds,W,-1);
      for(i=0;i<W;i++){ //diavasma oti egrapsan oi workers
       if(ret==-1)
        perror("poll");
       if(fds[i].revents & POLLIN){
      //   printf("pipe is readable\n");
         n=read(read_fd[i],&wc_temp,sizeof(int));//diavasma kai a8roisma bytes
         wc_bytes+=wc_temp;
         n=read(read_fd[i],&wc_temp,sizeof(int));//diavasma kai a8roisma words
         wc_words+=wc_temp;
         n=read(read_fd[i],&wc_temp,sizeof(int));//diavasma kai a8roisma lines
         wc_lines+=wc_temp;
        }
      }
      printf("WC result:total_bytes=%d,total_words=%d,total_lines=%d\n",wc_bytes,wc_words,wc_lines);
    }
    else if(flag==5){//exit
      free(command_buffer);
      for(j=0;j<W;j++){
        if ( unlink(fifo_read[j]) < 0) {
          perror("executor: can't unlink \n");
        }
        if ( unlink(fifo_write[j]) < 0) {
          perror("executor: can't unlink \n");
        }

      }
      break;
    }

}
  for(int i=0;i<W;i++){//kleisimo named pipes
    close(read_fd[i]);
    close(write_fd[i]);
  }
      for(int i=0;i<W;i++) // loop will run W times
        wait(NULL);
    }
    exit(1);
}
