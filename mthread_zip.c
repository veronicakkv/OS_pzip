#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define CHUNK_SIZE 16384
//== Chunk element for linked list
typedef struct CHUNK 
{
  char          *mem;
  int           size;
  //== you can add more chunk control variables here if it helps (like, output length, state of process, etc)
  struct CHUNK  *next;
} chunk_t;

//== Actual linked list: keeps track of all chunks and the order entered
typedef struct 
{
  chunk_t *first;
  chunk_t *last;
  int     numentries;
} chunklist;

chunklist list = { NULL, NULL, 0 }; // Define the list and initialize it

//== Function to add a chunk to the linked list
//== Returns: 1=success, 0=failed allocating chunk
int add_chunk_to_list(chunk_t *newchunk)
{
  //== Allocate a new chunk control structure & copy the data from caller
  chunk_t *n = malloc(sizeof(chunk_t));
  if(n == NULL) return(0);
  memcpy(n, newchunk, sizeof(chunk_t));
  
  if(list.first == NULL) list.first = n; else list.last->next = n;
  
  list.last = n;
  list.last->next = NULL;   // Ensure this indicates 'no more chunks following'
  list.numentries++;        // Not actually needed but useful for debugging
  
  return(1);
}

//== Function to open file, map it and create the required chunks to be processed
//== Returns: 1=success, 0=failed creating file chunks
int create_file_chunks(char *filename, int chunksize)
{
  FILE          *fp;
  char          *buff;
  chunk_t       chunk;
  struct stat   st;
  
  //== Get file info
  if(stat(filename,&st) == -1) { printf("zip: stat failed\r\n"); return(0); }
  
  //== Open file
  fp = fopen(filename, "rb");
  if(fp == NULL) { printf("zip: open failed\r\n"); return(0); }
    
  //== Read directly into chunks
  int actualchunksize;
  for(int processed=0; processed < st.st_size; processed += actualchunksize) 
  {
    int dataremain = (st.st_size - processed);
    
    //== Calc chunk size to allow 'irregular' file sizes (not multiples of 'chunksize')
    actualchunksize = dataremain;
    if(actualchunksize > chunksize) actualchunksize = chunksize;
    
    //== Allocate chunk memory & read it from the 
    buff = malloc(actualchunksize);
    if(buff == NULL) { fclose(fp); printf("zip: memory allocation failed\r\n"); return(0); }
    
    //== Read one chunk from file and add it to the linked list
    if(!fread(buff, actualchunksize, 1, fp)) { printf("zip: fread failed\r\n"); fclose(fp); return(0); }
    chunk.mem  = buff;
    chunk.size = actualchunksize;
    if(!add_chunk_to_list(&chunk)) { fclose(fp); return(0); }
  }
  
  fclose(fp);
  return(1);
}

//== Program entry point
int main(int argc, char *argv[])
{
  int i;
  
  //== Test command line args (at least one file to be specified besides the program name itself)
  if(argc < 2) { printf("pzip: file1 [file2 ...]\n"); exit(1); }
  
  //== Loop load all files to chunks (starting index = 1, to avoid the actual program name)
  for(i=1; i<argc; i++) {
    char *filename = argv[i];
    if(!create_file_chunks(filename, CHUNK_SIZE)) { printf("zip: failed processing file [%s]\n", filename); exit(1); }
  }
  
  ///////////////////////////////////////////////////////////////// YOUR CODE HERE TO HANDLE COMPRESSION
  
  //== To traverse the chunklist example:
  chunk_t *chunk;
  for(chunk = list.first; chunk != NULL; chunk = chunk->next) {
      //process_chunk(chunk); // <-- do what u want with this chunk
  }
  /////////////////////////////////////////////////////////////////
  
 return 0; 
}





