
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define MAX_LINE 63
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
  Node* (*peek) (struct Queue*);
  sem_t max;
  sem_t start;
} Queue;

int push (Queue* q, char *line);
char* pop (Queue* q);
Node* peek (Queue* q);

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
    n->item = malloc(MAX_LINE + 1);
    if (strlen(line) > MAX_LINE) {
      line[MAX_LINE - 1] = '\n';
      strncat(n->item, line, MAX_LINE);
    } else {
      strcpy(n->item, line);
    }
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
  q->head = q->head->next;
  q->size--;
  return item;
}

Node * peek(Queue* q) {
  return q->head;
}

void * do_producer() {
  char *line = malloc(MAX_LINE);
  size_t size;
  threads = 0;

  while (getline(&line, &size, stdin) > -1) {
    threads++;
    while (!q1.push(&q1, line)) {
      sem_wait(&q1.max);
    }
    sem_post(&q1.start);
  }
  done = 1;
}

void * do_crunch() {
  int i = 0;
  while (i < threads || !done) {
    while (!q1.peek(&q1)) {
      sem_wait(&q1.start);
    }
    char *line = q1.pop(&q1);
    char *s;
    sem_post(&q1.max);

    s = strchr(line, ' ');
    while (s != NULL) {
      line[s-line] = '*';
      s = strchr(s+1, ' ');
    }
    while (!q2.push(&q2, line)) {
      sem_wait(&q2.max);
    }
    sem_post(&q2.start);
    i++;
  }
}

void * do_gobble() {
  int c = 0;
  while (c < threads || !done) {
    while (!q2.peek(&q2)) {
      sem_wait(&q2.start);
    }
    char *line = q2.pop(&q2);
    sem_post(&q2.max);

    int i = 0;
    while (line[i] != '\0') {
      line[i] = toupper(line[i]);
      i++;
    }
    while (!q3.push(&q3, line)) {
      sem_wait(&q3.max);
    }
    sem_post(&q3.start);
    c++;
  }
}

void * do_consumer() {
  int c = 0;
  while (c < threads || !done) {
    while (!q3.peek(&q3)) {
      sem_wait(&q3.start);
    }
    char *line = q3.pop(&q3);
    sem_post(&q3.max);
    printf("%s", line);
    free(line);
    c++;
  }
  printf("\n\nTotal lines: %d", threads);
  printf("\n");
}

int main(int argc, char **argv) {
  q1.size = 0;
  q1.head = NULL;
  q1.tail = NULL;
  q1.push = &push;
  q1.pop = &pop;
  q1.peek = &peek;
  sem_init(&q1.start, 0, 0);
  sem_init(&q1.max, 0, 0);

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

  sem_destroy(&q1.max);
  sem_destroy(&q1.start);
  sem_destroy(&q2.max);
  sem_destroy(&q2.start);
  sem_destroy(&q3.max);
  sem_destroy(&q3.start);

  return 0;
}
