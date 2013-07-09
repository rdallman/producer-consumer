
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define ROW 10
#define COL 63

int threads;

typedef struct Node {
  char *item;
  struct Node* next;
} Node;

typedef struct Queue {
  int size;
  Node* head;
  Node* tail;

  void (*push) (struct Queue*, char*);
  char* (*pop) (struct Queue*);
} Queue;

typedef struct Params {
  struct Queue* q;
  struct Node* node;
} Params;


void push (Queue* q, char *line);
char* pop (Queue* q);

void * do_crunch();
void * do_gobble();
void * do_consumer();

void push (Queue* q, char *line) {
  Node* n = (Node*) malloc (sizeof(Node));
  n->item = line;
  n->next = NULL;
  if (q->head == NULL) {
    q->head = n;
  } else {
    q->tail->next = n;
  }
  q->tail = n;
  q->size++;
}

char * pop(Queue* q) {
  char *item = q->head->item;
  Node* head = q->head;
  q->head = q->head->next;
  q->size--;
  free(head);
  return item;
}

void * do_producer() {
  char *line = (char*)malloc(COL * sizeof(char));
  size_t size;
  int i = 0;
  threads = 0;
  pthread_t crunch;
  Queue q;
  q.size = 0;
  q.head = NULL;
  q.tail = NULL;
  q.push = &push;
  q.pop = &pop;

  while (getline(&line, &size, stdin) > -1) {

    q.push(&q, line);
    //printf("%s", q.pop(&q));
    Params p;
    p.q = &q;
    p.node = q.tail;
    if(pthread_create(&crunch, NULL, &do_crunch, &p)) {
      printf("Couldn't create thread\n");
    }

    /*
    if (q.size = ROW) {
      if(pthread_join(q.pop, NULL))
      {
        printf("Could not join thread\n");
      }
    }
    */

    i++;
    threads++;
    printf("%d", threads);

  }
  printf("\n\nTotal lines: %d", i);
}

void * do_crunch(void *args) {
  Params *p = args;
  char *line = p->node->item;

  char *s;

  s = strchr(line, ' ');
  while (s != NULL) {
    line[s-line] = '*';
    s = strchr(s+1, ' ');
  }
  pthread_t gobble;
  if(pthread_create(&gobble, NULL, &do_gobble, p)) {
    printf("Couldn't create thread\n");
  }

  if(pthread_join(gobble, NULL))
  {
    printf("Could not join thread\n");
  }
}

void * do_gobble(void *args) {
  Params *p = args;
  char *line = p->node->item;

  int i = 0;
  while (line[i] != '\0') {
    line[i] = toupper(line[i]);
    i++;
  }
  pthread_t consumer;
  if(pthread_create(&consumer, NULL, &do_consumer, p)) {
    printf("Couldn't create thread\n");
  }

  if(pthread_join(consumer, NULL))
  {
    printf("Could not join thread\n");
  }
}

void * do_consumer(void *args) {
  Params *p = args;
  Queue *q = p->q;
  /* fuck c
  char* line = p->q.pop(&q);
  printf("%s", line);
  */
  threads--;
}

int main(int argc, char **argv) {
  //char *b[ROW];

  /*
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
  */

  pthread_t producer;

  if (pthread_create(&producer, NULL, &do_producer, NULL)) {
    printf("Could not create thread \n");
    return -1;
  }
  if (pthread_join(producer, NULL)) {
    printf("Could not join thread \n");
    return -1;
  }
  //this needs to change
  /*
  for (i=0; i<ROW; i++) {
    free(b[i]);
  }
  */

  return 0;
}

