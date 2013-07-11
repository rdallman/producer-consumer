
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define ROW 10
#define COL 63
#define MAX_QUEUE 10

int done;

typedef struct Node {
  char *item;
  struct Node* next;
} Node;

typedef struct Queue {
  int size;
  Node* head;
  Node* tail;

  int (*push) (struct Queue*, char*);
  char* (*pop) (struct Queue*);
  char* (*peek) (struct Queue*);
} Queue;

int push (Queue* q, char *line);
char* pop (Queue* q);
char* peek (Queue* q);

void * do_producer();
void * do_crunch();
void * do_gobble();
void * do_consumer();

int done;
int threads;
Queue q1;
Queue q2;
Queue q3;


int push (Queue* q, char *line) {
  if (q->size < MAX_QUEUE) {
    Node *n = (Node*)malloc (sizeof(Node));
    n->item = malloc(63 + 1);
    if (strlen(line) > 63) {
      /*
      int i;
      for (i = 0; i < 63; i++) {
        n->item[i] = line[i];
      }
      n->item[63] = '\0';
      */
      line[62] = '\n';
      strncat(n->item, line, 63);
    } else {
      strcpy(n->item, line);
    }

    //strcpy(n->item, line);
    n->next = NULL;
    if (!q->head) {
      q->head = n;
    } else {
      q->tail->next = n;
    }
    q->tail = n;
    q->size++;
    return 1;
  } else {
    return 0;
  }
}

char * pop(Queue* q) {
  char* item = q->head->item;
  if (!q->head->next) {
    q->head = NULL;
  } else {
    q->head = q->head->next;
  }
  q->size--;
  return item;
}

char * peek(Queue* q) {
  if (q->head) {
    return 1;
  } else {
    return 0;
  }
}

void * do_producer() {
  char *line = malloc(COL);
  size_t size;
  int i = 0;
  threads = 0;

  while (getline(&line, &size, stdin) > -1) {

    threads++;
    //q1.push(&q1, line);
    while (!q1.push(&q1, line)) {;}

    //printf("%s", q1.pop(&q1));
    //printf("%d", q1.size);

    i++;
    //printf("%d", threads);

  }
  done = 1;
  printf("\n\nTotal lines: %d", i);
}

void * do_crunch() {
  int i = 0;
  while (i < threads || !done) {
    //printf("%s", q1.pop(&q1));
    while (!q1.peek(&q1)) {;}
    char *line = q1.pop(&q1);
    char *s;

    s = strchr(line, ' ');
    while (s != NULL) {
      line[s-line] = '*';
      s = strchr(s+1, ' ');
    }
    //q2.push(&q2, line);
    while (!q2.push(&q2, line)) {;}
    i++;
  }
}

void * do_gobble() {
  int c = 0;
  while (c < threads || !done) {
    while (!q2.peek(&q2)) {;}
    char *line = q2.pop(&q2);

    int i = 0;
    while (line[i] != '\0') {
      line[i] = toupper(line[i]);
      i++;
    }
    //q3.push(&q3, line);
    while (!q3.push(&q3, line)) {;}
    c++;
  }
}

void * do_consumer() {
  int c = 0;
  while (c < threads || !done) {
    while (!q3.peek(&q3)) {;}
    char *line = q3.pop(&q3);
    printf("%s", line);
    free(line);
    c++;
  }
}

//bad

int main(int argc, char **argv) {
  q1.size = 0;
  q1.head = NULL;
  q1.tail = NULL;
  q1.push = &push;
  q1.pop = &pop;
  q1.peek = &peek;

  q2 = q1;
  q3 = q1;
  pthread_t producer;
  pthread_t crunch;
  pthread_t gobble;
  pthread_t consumer;
  done = 0;

  if (pthread_create(&producer, NULL, &do_producer, NULL)) {
    printf("Could not create thread \n");
  }
  if(pthread_create(&crunch, NULL, &do_crunch, NULL)) {
    printf("Couldn't create thread\n");
  }
  if(pthread_create(&gobble, NULL, &do_gobble, NULL)) {
    printf("Couldn't create thread\n");
  }
  if(pthread_create(&consumer, NULL, &do_consumer, NULL)) {
    printf("Couldn't create thread\n");
  }


  if (pthread_join(producer, NULL)) {
    printf("Could not join thread \n");
    return -1;
  }
  if(pthread_join(crunch, NULL))
  {
    printf("Could not join thread\n");
  }
  if(pthread_join(gobble, NULL))
  {
    printf("Could not join thread\n");
  }
  if(pthread_join(consumer, NULL))
  {
    printf("Could not join thread\n");
  }

  return 0;
}
