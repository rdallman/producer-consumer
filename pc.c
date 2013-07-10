
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define ROW 10
#define COL 63


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
  char* (*peek) (struct Queue*);
} Queue;

void push (Queue* q, char *line);
char* pop (Queue* q);
char* peek (Queue* q);

void * do_producer();
void * do_crunch();
void * do_gobble();
void * do_consumer();

int threads;
Queue q1;
Queue q2;
Queue q3;


void push (Queue* q, char *line) {
  Node *n = (Node*)malloc (sizeof(Node));
  n->item = malloc(strlen(line) + 1);
  strcpy(n->item, line);
  n->next = NULL;
  if (!q->head) {
    q->head = n;
  } else {
    q->tail->next = n;
  }
  q->tail = n;
  q->size++;
}

char * pop(Queue* q) {
  char* item = malloc(COL * sizeof(char) + 1);
  strcpy(item, q->head->item);
  q->head = q->head->next;
  q->size--;
  return item;
}

char * peek(Queue* q) {
  Node* head = q->head;
  char* item = malloc(strlen(head->item) + 1);
  strcpy(item, head->item);
  return item;
}

void * do_producer() {
  char *line = (char*)malloc(COL * sizeof(char));
  size_t size;
  int i = 0;
  threads = 0;

  while (getline(&line, &size, stdin) > -1) {

    q1.push(&q1, line);
    //printf("%s", q1.pop(&q1));
    printf("%d", q1.size);

    i++;
    threads++;
    printf("%d", threads);

  }
  printf("\n\nTotal lines: %d", i);
}

void * do_crunch() {
  sleep(2);
  while (1) {
    //printf("%s", q1.pop(&q1));
    char *line = q1.pop(&q1);
    char *s;

    s = strchr(line, ' ');
    while (s != NULL) {
      line[s-line] = '*';
      s = strchr(s+1, ' ');
    }
    printf ("%s", line);
    q2.push(&q2, line);
  }
}

void * do_gobble() {
  /*
  char *line = q2.pop(&q2);

  int i = 0;
  while (line[i] != '\0') {
    line[i] = toupper(line[i]);
    i++;
  }
  q3.push(&q3, line);
  */
}

void * do_consumer() {
  /*
  char *line = q3.pop(&q3);
  printf("%s", line);
  threads--;
  */
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

  if (pthread_create(&producer, NULL, &do_producer, NULL)) {
    printf("Could not create thread \n");
    return -1;
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
