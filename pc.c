
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define THREADS 10
#define ROW 10
#define COL 63

pthread_barrier_t barr;

void * do_crunch();
void * do_gobble();
void * do_consumer();

void * do_producer(char *line) {
  //do some stuff
  //printf("%s", line);
  pthread_t crunch;
  if(pthread_create(&crunch, NULL, &do_crunch, line)) {
    printf("Couldn't create thread\n");
  }

  if (pthread_join(crunch, NULL)) {
    printf("Could not join thread\n");
  }

  int rc = pthread_barrier_wait(&barr);
  if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
    printf("Could not wait on barrier\n");
    exit(-1);
  }
}

void * do_crunch(char *line) {
  //do some stuff
  char *s;
  //printf("%s", line);

  s = strchr(line, ' ');
  while (s != NULL) {
    line[s-line] = '*';
    s = strchr(s+1, ' ');
  }
  pthread_t gobble;
  if(pthread_create(&gobble, NULL, &do_gobble, line)) {
    printf("Couldn't create thread\n");
  }

  if(pthread_join(gobble, NULL))
  {
    printf("Could not join thread\n");
  }
}

void * do_gobble(char *line) {
  //do some stuff

  int i = 0;
  while (line[i] != '\0') {
    line[i] = toupper(line[i]);
    i++;
  }
  //printf("%s", line);
  pthread_t consumer;
  if(pthread_create(&consumer, NULL, &do_consumer, line)) {
    printf("Couldn't create thread\n");
  }

  if(pthread_join(consumer, NULL))
  {
    printf("Could not join thread\n");
  }
}

void * do_consumer(char *line) {
  FILE *file;
  file = fopen("output.txt", "a+");
  fprintf(file, "%s", line);
  fclose(file);
}

int main(int argc, char **argv) {
  char *b[ROW];

  int i;
  //b
  for (i=0; i<ROW; i++) {
    if ((b[i] = malloc(sizeof(char) * COL)) == NULL) {
      printf("unable to allocate memory \n");
      return -1;
    }
  }

  for (i=0; i<ROW; i++) {
    char line = NULL;
    size_t size;
    if (fgets(b[i], COL, stdin)) {
      //bad
    } else {
    }
  }
  printf(b[0]);



  pthread_t producer[THREADS];

  if(pthread_barrier_init(&barr, NULL, THREADS)) {
    printf("Could not create a barrier\n");
    return -1;
  }
  for (i = 0; i < THREADS; ++i) {
    if (pthread_create(&producer[i], NULL, &do_producer, b[i])) {
      printf("Could not create thread %d\n", i);
      return -1;
    }
  }
  for (i = 0; i < THREADS; ++i) {
    if (pthread_join(producer[i], NULL)) {
      printf("Could not join thread %d\n", i);
      return -1;
    }
  }
  //this needs to change
  for (i=0; i<ROW; i++) {
    free(b[i]);
  }

  return 0;
}