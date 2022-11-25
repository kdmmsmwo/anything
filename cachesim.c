#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
 
typedef struct cacheline{
  bool dirty;
  bool valid;
  int tag;
  int* data;
  int writecount;
}cacheline;

typedef struct cacheindex{
  int writecount;
  cacheline* block;
}cacheindex;

typedef struct memlist{
  int adress;
  int* data;
  struct memlist* next;
}memlist;


int logtwo(int x){
  int n = 0;
  while (x != 1){
    x = x/2;
    n++;
  }
  return n;
}

int sr(int x, int n){
  if (x<0) {return ((x>>n) & (~(0xFFFFFFFF << (32-n))));}
  else{return x>>n;}
}

int optoint(char* op){
  int oplen = strlen(op);
  int value = 0;
  for (int i = 3 ; i < oplen ; i++){
    value = value*10;
    value += ((int)op[i]) - 48;
   }
  return value;
}

char* optofname(char* op){
  char* fname = (char*) malloc(sizeof(char)*50);
  int len = strlen(op);
  for(int i = 3;i<len;i++){
    fname[i-3] = op[i];
  }
  return fname;
}

int hextodec(char* hex){
  int value = 0;
  int len = strlen(hex);

  for (int i = 0; i< len ; i++){
    value = value * 16;
    if ( (48<=hex[i]) & (hex[i]<=57)){value += ((int)hex[i]) - 48;}
    else {value += ((int)hex[i]) - 55;}
  }
  return value;
}

int strtoint(char* str){
  int value = 0;
  int len = strlen(str);
  if (str[0] == '-'){
    for (int i = 1; i< len-1 ; i++){
      value = value * 10;
      value += ((int)str[i]) - 48;
    }
    value = value * (-1);
  }
  else{
    for (int i = 0; i< len-1 ; i++){
      value = value * 10;
      value += ((int)str[i]) - 48;
    }
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

cacheindex* initcache(int bsize, int ssize, int idx){
  cacheindex* cache = (cacheindex*) malloc(sizeof(cacheindex)*idx);
  for (int i = 0; i<idx ; i++){
    cache[i].writecount = -1;
    cache[i].block = createline(ssize, bsize);
  }
  return cache;
}

int getoldlinenum(cacheindex cache, int ssize){
  int oldline = -1;
  int oldcount = 1000000;
  for(int i = 0; i < ssize ; i++){
    if (cache.block[i].writecount < oldcount){
      oldline = i;
      oldcount = cache.block[i].writecount;
    }
  }
  return oldline;
}

void printcache(cacheindex* cache, int bsize, int ssize, int idx){
  int datasize = bsize/4;
  for (int i = 0 ; i<idx ; i++){
    printf("%d: ",i);
    for (int k = 0 ; k<ssize ; k++){
      if(k != 0){printf("   ");}
      for (int j = 0;j<datasize;j++){
        //printf("%s ",intobin(cache[i].block[k].data[j]));
        printf("%.8x ",cache[i].block[k].data[j]);
      }
      printf("v:%d, d:%d\n",cache[i].block[k].valid,cache[i].block[k].dirty);
    }
  }
}

int getdirtycount(cacheindex* cache, int ssize, int idx){
  int dirtycount = 0;
  for (int i = 0 ; i<idx ; i++){
    for (int k = 0 ; k<ssize ; k++){
      if (cache[i].block[k].dirty == true){ dirtycount++; }
    }
  }
  return dirtycount;
}

memlist* createmem(int adr, int* dat, int datacount){
  memlist* mem = (memlist*) malloc(sizeof(memlist));
  mem->adress = adr;
  int* tmp = (int*)malloc(sizeof(int)*datacount);
  for (int i = 0;i<datacount;i++){
    tmp[i] = dat[i];
  }
  mem->data = tmp;
  mem->next = NULL;
  return mem;
}

void memappend(memlist* first, int adr, int* dat, int datacount){
  memlist* tmp = first;
  while(tmp->next != NULL){tmp = tmp->next;}
  tmp->next = createmem(adr, dat, datacount);
  first->data[0] += 1;
}

memlist* findmem(memlist* first, int adr){
  memlist* tmp = first;
  for (int i =0;i<first->data[0];i++){
    if (tmp->next != NULL){tmp = tmp->next;}
    if (tmp->adress == adr){return tmp;}
  }
  tmp = NULL;
  return tmp;
}

void printmem(memlist* first, int datacount){
  int count = first->data[0];
  printf("count : %d\n", count);
  memlist* tmp = first->next;
  for(int i = 0;i<count;i++){
    printf("node : %d  ",i+1);
    printf("data : %.8x  ",tmp->data[0]);
    for (int k = 1; k<datacount;k++){printf("%.8x  ",tmp->data[k]);}
    printf("adress : %.8x\n",tmp->adress);
    tmp = tmp->next;
  }
}

int main(int argc, char* argv[]){
  int hitcount;
  int misscount;
  int cachesize;
  int blocksize;
  int setsize;
  char* fname;
  
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
    else if (argv[i][1] == 'f'){
      fname = optofname(argv[i]);
    }
  }

  FILE* trace = fopen(fname,"r");
  if (trace == NULL) {
    printf("open failure");
    return 0;
    }

  int datacount = blocksize/4;
  int databit = logtwo(blocksize);
  int idx = ((cachesize / blocksize) / setsize);
  cacheindex* cache = initcache(blocksize, setsize, idx);
  int t[1] = {0};
  memlist* first = createmem(-1,t,datacount);

  char str[50];
  fgets(str,50,trace);
  while (strlen(str) > 9){
    int* line = linetoint(str);
    int index = sr(line[0], databit) % idx;
    int tg = sr(line[0], databit) / idx;
    int offset = sr((line[0] << (32-databit)),32-databit);

    if (line[1] == 1){ // write
      hitcount++;
      int sametag = 0;
      for (int i = 0;i<setsize;i++){
        if (cache[index].block[i].tag == tg){
          sametag = 1;
          cache[index].block[i].data[offset/4] = line[2];
          cache[index].block[i].dirty = 1;
          cache[index].block[i].valid = 1;
          cache[index].block[i].writecount = cache[index].writecount;
        }
      }
      if (sametag != 1){
        int oldline = getoldlinenum(cache[index],setsize);
        if (cache[index].block[oldline].dirty == true){ //spill
          hitcount--;
          misscount++;
          int* apdata = cache[index].block[oldline].data;
          int apadr = (cache[index].block[oldline].tag * idx) + index;
          memlist* readmem = findmem(first, apadr);
          if (readmem == NULL){ memappend(first,apadr,apdata,datacount); }
          else { readmem->data = apdata; }
        }
        
        cache[index].block[oldline].data[offset/4] = line[2];
        cache[index].block[oldline].dirty = true;
        cache[index].block[oldline].tag = tg;
        cache[index].block[oldline].valid = true;
        cache[index].block[oldline].writecount = cache[index].writecount;
      }
      cache[index].writecount++;
    }
    else if (line[1] == 0){ // read
      int ishit = 0;
      for(int i = 0; i<setsize;i++){
        if ((cache[index].block[i].tag == tg) & (cache[index].block[i].valid == true)){ // hit
          ishit = 1;
          hitcount++;
          break;
        }
      }
      if (ishit == 0){ // miss
        misscount++;
        int oldline = getoldlinenum(cache[index],setsize);
        if (cache[index].block[oldline].dirty == true){ //spill
          int* apdata = cache[index].block[oldline].data;
          int apadr = (cache[index].block[oldline].tag * idx) + index;
          memlist* readmem = findmem(first, apadr);
          if (readmem == NULL){ memappend(first,apadr,apdata,datacount); }
          else { readmem->data = apdata; }
        }
        cache[index].block[oldline].dirty = false;
        cache[index].block[oldline].valid = true;
        cache[index].block[oldline].tag = tg;

        memlist* readmem = findmem(first, sr(line[0],databit));
        if (readmem == NULL){
          for (int i = 0;i<datacount;i++){
            cache[index].block[oldline].data[i] = 0;
          }
        }
        else{
          for (int i = 0;i<datacount;i++){
            cache[index].block[oldline].data[i] = readmem->data[i];
          }
        }
        cache[index].writecount++;
      }
    }
    fgets(str,50,trace);
  }

  printcache(cache, blocksize, setsize, idx);
  //printmem(first, datacount);
  float missrate = ((float)misscount / ((float)hitcount+(float)misscount)) * 100;
  float averagecycle = (((float)misscount*201.0) + (float)hitcount)/ ( (float)misscount+(float)hitcount );

  printf("total number of hits: %d\n",hitcount);
  printf("total number of misses: %d\n",misscount);
  printf("miss rate: %.1f%%\n",missrate);
  printf("total number of dirty blocks: %d\n",getdirtycount(cache,setsize,idx));
  printf("average memory access cycle: %.1f\n", averagecycle);

  fclose(trace);
  return 0;
}
