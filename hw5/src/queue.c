#include "queue.h"
#include <errno.h>

queue_t *create_queue(void) {
    int size = 1;
    queue_t *NewQueue = calloc(size, sizeof(queue_t));
    NewQueue->invalid = false;
    if(NewQueue == NULL){
        return NULL;
    }
    sem_init(&NewQueue->items, 0, 0);
    if(pthread_mutex_init(&NewQueue->lock, NULL) != 0){
        return NULL;
    }
    return NewQueue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    if(self->invalid == true || self== NULL || destroy_function == NULL){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->lock);
    queue_node_t *Current = self->front;
    while(Current != NULL){
    destroy_function(Current->item);
    queue_node_t *Next = Current->next;
    free(Current);
    Current = Next;
    }
    self->invalid = true;;
    pthread_mutex_unlock(&self->lock);
    return true;
}

bool enqueue(queue_t *self, void *item) {
    if(self->invalid == true || self == NULL || item == NULL){
        errno = EINVAL;
        return false;
    }
    int size = 1;
    queue_node_t *NewNode = calloc(size, sizeof(queue_node_t));
    if(NewNode == NULL){
        return false;
    }
    pthread_mutex_lock(&self->lock);
    NewNode->item = item;
    queue_node_t *rear = self->rear;
    if(rear == NULL){
        self->front = NewNode;
        rear = NewNode;
    }
    else{
    rear->next = NewNode;
    self->rear = NewNode;
    }
    pthread_mutex_unlock(&self->lock);
    sem_post(&self->items);
    return true;
}

void *dequeue(queue_t *self) {
    if(self->invalid == true || self == NULL){
        errno = EINVAL;
        return false;
    }
    sem_wait(&self->items);
    pthread_mutex_lock(&self->lock);
    queue_node_t *front = self->front;
    if(front != NULL){
    void * item = front->item;
    self->front = front->next;
    free(self->front);
    pthread_mutex_unlock(&self->lock);
    return item;
    }
    else{
        pthread_mutex_unlock(&self->lock);
        return NULL;
    }
}
