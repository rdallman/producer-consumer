
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
  pthread_mutex_t mutex;
  pthread_cond_t max;
  pthread_cond_t start;

  void* (*push) (struct Queue*, char*);
  char* (*pop) (struct Queue*);
  struct Queue* (*init) ();
} Queue;

void* push (Queue* q, char *line);
char* pop (Queue* q);
Queue* init ();

void * do_producer();
void * do_crunch();
void * do_gobble();
void * do_consumer();

int done;
int threads;
Queue* q1;
Queue* q2;
Queue* q3;


void * push (Queue* q, char *line) {
    Node *n = (Node*)malloc (sizeof(Node));
    n->item = line;
    n->next = NULL;
    /*
    if (strlen(line) > MAX_LINE) {
      line[MAX_LINE - 1] = '\n';
      strncat(n->item, line, MAX_LINE);
    } else {
      strcpy(n->item, line);
    }
    */

    if (q->head == NULL) {
      q->head = n;
    } else {
      q->tail->next = n;
    }
    q->tail = n;
    q->size++;
}

char * pop(Queue* q) {
  Node* head = q->head;
  char *item = head->item;
  q->head = head->next;
  //free(&head);
  q->size--;
  return item;
}

Queue* init() {
  Queue* q;
  q = (Queue*)malloc(sizeof(Queue));
  q->size = 0;
  q->head = NULL;
  q->tail = NULL;
  q->push = &push;
  q->pop = &pop;
  pthread_mutex_init(&q->mutex, NULL);
  return q;
}

void * do_producer() {
  char *line = malloc(MAX_LINE);
  size_t size;
  threads = 0;

  while (getline(&line, &size, stdin) > -1) {
    pthread_mutex_lock(&q1->mutex);
    while (q1->size >= MAX_QUEUE) {
      pthread_cond_wait(&q1->max, &q1->mutex);
    }
    printf("%d", q1->size);
    threads++;
    q1->push(q1, line);
    pthread_cond_signal(&q1->start);
    pthread_mutex_unlock(&q1->mutex);
  }
  done = 1;
  printf("\n\nTotal lines: %d", threads);
}

void * do_crunch() {
  int i = 0;
  while (i < threads || !done) {
    pthread_mutex_lock(&q1->mutex);
    while (q1->size <= 0)  {
      pthread_cond_wait(&q1->start, &q1->mutex);
    }
    char *line = q1->pop(q1);

    pthread_cond_signal(&q1->max);
    pthread_mutex_unlock(&q1->mutex);

    char *s;

    s = strchr(line, ' ');
    while (s != NULL) {
      line[s-line] = '*';
      s = strchr(s+1, ' ');
    }
    printf("%s", line);

    pthread_mutex_lock(&q2->mutex);
    while (q2->size >= MAX_QUEUE) {
      pthread_cond_wait(&q2->max, &q2->mutex);
    }
    printf("%d", q2->size);
    q2->push(q2, line);
    i++;
    pthread_cond_signal(&q2->start);
    pthread_mutex_unlock(&q2->mutex);
  }
}

void * do_gobble() {
  int c = 0;
  while (c < threads || !done) {
    pthread_mutex_lock(&q2->mutex);
    while (q2->size <= 0) {
      pthread_cond_wait(&q2->start, &q2->mutex);
    }
    char *line = q2->pop(q2);
    pthread_cond_signal(&q2->max);
    pthread_mutex_unlock(&q2->mutex);

    int i = 0;
    while (line[i] != '\0') {
      line[i] = toupper(line[i]);
      i++;
    }

    pthread_mutex_lock(&q3->mutex);
    while (q3->size >= MAX_QUEUE) {
      pthread_cond_wait(&q3->max, &q3->mutex);
    }
    printf("%d", q3->size);
    q3->push(q3, line);
    c++;
    pthread_cond_signal(&q3->start);
    pthread_mutex_unlock(&q3->mutex);
  }
}

void * do_consumer() {
  int c = 0;
  while (c < threads || !done) {
    pthread_mutex_lock(&q3->mutex);
    while (q3->size <= 0) {
      pthread_cond_wait(&q3->start, &q3->mutex);
    }
    char *line = q3->pop(q3);
    printf("%s", line);
    //free(line);
    c++;
    pthread_cond_signal(&q3->max);
    pthread_mutex_unlock(&q3->mutex);
  }
}

//bad

int main(int argc, char **argv) {
  q1 = init();
  q2 = init();
  q3 = init();

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
