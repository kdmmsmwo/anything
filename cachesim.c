#include <stdio.h>
#include <String.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
 
typedef struct cacheline{
  bool dirty;
  bool valid;
  int tag;
  int* data;
}cacheline;

typedef struct cacheindex{
  int writecount;
  cacheline* block;
}cacheindex;



int optoint(char* op){
  int oplen = strlen(op);
  int value = 0;

  for (int i = 3 ; i < oplen ; i++){
    value = value*10;
    value += ((int)op[i]) - 48;
   }

  return value;
}

int hextodec(char* hex){
  int value = 0;
  int len = strlen(hex);

  for (int i = 0; i< len ; i++){
    value = value * 2;
    value += ((int)hex[i]) - 48;
  }

  return value;
}

int strtoint(char* str){
  int value = 0;
  int len = strlen(str);

  for (int i = 0; i< len-1 ; i++){
    value = value * 10;
    value += ((int)str[i]) - 48;
  }

  return value;
}

int* linetoint(char* line){
  int* result = (int*)malloc(sizeof(int)*3);
  char* tok = (char*)malloc(sizeof(char)*10);
  char* space = (char*)malloc(sizeof(char)*2);
  strcpy(space," ");
  tok = strtok(line,space);
  result[0] = hextodec(tok);
  tok = strtok(NULL,space);
  if (tok[0] == 'W'){
    result[1] = 1;

    tok = strtok(NULL,space);
    result[2] = strtoint(tok);
  }
  else{
    result[1] = 0;
    result[2] = 0;
    }
  tok = strtok(NULL,space);

  free(space);
  free(tok);
  return result;
}

cacheline* createline(int ssize, int bsize){
  int datacount = bsize/4;
  cacheline* line = (cacheline*) malloc(sizeof(cacheline)*ssize);
  for(int i = 0;i<ssize;i++){
    line[i].data = (int*)malloc(sizeof(int)*datacount);
    for(int k=0;k<datacount;k++){line[i].data[k] = 0;}
    line[i].dirty = false;
    line[i].valid = false;
    line[i].tag = 0;
  }
  return line;
}

cacheindex* initcache(int csize, int bsize, int ssize, int idx){

  cacheindex* cache = (cacheindex*) malloc(sizeof(cacheindex)*idx);
  for (int i = 0; i<idx ; i++){
    cache[i].writecount = 0;
    cache[i].block = createline(ssize, bsize);
  }

  return cache;
}

void printcache(cacheindex* cache, int bsize, int ssize, int idx){
  int datasize = bsize/4;
  for (int i = 0 ; i<idx ; i++){
    printf("%d: ",i);
    for (int k = 0 ; k<ssize ; k++){
      
      for (int j = 0;j<datasize;j++){
        printf("%d ",cache[i].block[k].data[j]);
      }
      printf(" v:%d, d:%d\n",cache[i].block[k].valid,cache[i].block[k].dirty);

    }

  }

}

int main(int argc, char* argv[]){
  int hitcount = 0;
  int misscount = 0;
  int cachesize = 64;
  int blocksize=8;
  int setsize=2;
  FILE* trace = fopen("sample.trc","r");

  if (trace == NULL) {
    printf("open failure");
    return 0;
    }

  for (int i = 1; i<argc;i++){
    if (argv[i][1] == 's'){
      cachesize = optoint(argv[i]);
    }
    else if (argv[i][1] == 'b'){
      blocksize = optoint(argv[i]);
    }
    else if (argv[i][1] == 'a'){
      setsize = optoint(argv[i]);
    }
  }

  int idx = ((cachesize / blocksize) / setsize);
  char str[50];
  fgets(str,50,trace);
  printf("\n%s\n",str);

  int* line = linetoint(str);
  printf("\n%d %d %d\n",line[0],line[1],line[2]);
  
  cacheindex* cache = initcache(cachesize, blocksize, setsize, idx);
  printcache(cache, blocksize, setsize, idx);

  fclose(trace);
  return 0;
}

