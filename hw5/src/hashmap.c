#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if(capacity == 0 || hash_function == NULL || destroy_function == NULL){
        errno = EINVAL;
        return NULL;
    }
    int size = 1;
    hashmap_t *NewHashMap = calloc(size, sizeof(hashmap_t));
    if(NewHashMap == NULL){
        return NULL;
    }
    map_node_t *ArrayOfNodes = calloc(capacity,sizeof(map_node_t));
    if(ArrayOfNodes == NULL){
        return NULL;
    }
    NewHashMap->capacity = capacity;
    NewHashMap->size = 0;
    NewHashMap->nodes = ArrayOfNodes;
    NewHashMap->num_readers = 0;
    if(pthread_mutex_init(&NewHashMap->write_lock, NULL) != 0){
        return NULL;
    }
    if(pthread_mutex_init(&NewHashMap->fields_lock, NULL) != 0){
        return NULL;
    }
    NewHashMap->hash_function = hash_function;
    NewHashMap->destroy_function = destroy_function;
    return NewHashMap;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    if(self == NULL ){
        errno = EINVAL;
        return NULL;
    }
    if(key.key_base == NULL){
        key.key_base = malloc(sizeof(key.key_base));
    }
    if(val.val_base == NULL){
        val.val_base = malloc(sizeof(val.val_base));
    }
    if(self->capacity == self->size && force == false){
        errno = ENOMEM;
        return false;
    }
    //Get Current Node
    pthread_mutex_lock(&self->write_lock);
    map_node_t *ArrayOfNodes = self->nodes;
    int index = get_index(self, key);
    map_node_t *CurrentNode= ArrayOfNodes + index;
    //Place it
    if(self->capacity == self->size && force == true){
        destructor_f d_f = self->destroy_function;
        d_f(CurrentNode->key,CurrentNode->val);
        CurrentNode->key = key;
        CurrentNode->val = val;
        CurrentNode->tombstone = false;
    }
    else if(CurrentNode->key.key_base == NULL){
        CurrentNode->key = key;
        CurrentNode->val = val;
        CurrentNode->tombstone = false;
    }
    else if(CurrentNode->tombstone == true){
        CurrentNode->key = key;
        CurrentNode->val = val;
        CurrentNode->tombstone = false;
    }
    else{
        //Update if it exists
        if(CurrentNode->key.key_len == key.key_len && memcmp(CurrentNode->key.key_base,key.key_base,key.key_len) == 0){
            CurrentNode->val = val;
            pthread_mutex_unlock(&self->write_lock);
            return true;
        }
        int tempindex = index;
        CurrentNode++;
        index++;
        if(index > self->capacity){
            index = 0;
            CurrentNode = ArrayOfNodes;
        }
        while(index != tempindex){
            if(CurrentNode->key.key_base == NULL){
                CurrentNode->key = key;
                CurrentNode->val = val;
                CurrentNode->tombstone = false;
                self->size++;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
            else if(CurrentNode->tombstone == true){
                CurrentNode->key = key;
                CurrentNode->val = val;
                CurrentNode->tombstone = false;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
            else{
                if(CurrentNode->key.key_len == key.key_len && memcmp(CurrentNode->key.key_base,key.key_base,key.key_len) == 0){
                CurrentNode->val = val;
                pthread_mutex_unlock(&self->write_lock);
                return true;
                }
                index++;
                CurrentNode++;
            }
            if(index >= self->capacity){
                index = 0;
                CurrentNode = ArrayOfNodes;
            }
        }
    }
    self->size++;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    if(self == NULL){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if(self->num_readers == 1){
        pthread_mutex_lock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);
    map_node_t *ArrayOfNodes = self->nodes;
    int index = get_index(self,key);
    map_node_t *CurrentNode= ArrayOfNodes + index;
    if(CurrentNode->key.key_len == key.key_len && CurrentNode->tombstone != true && memcmp(CurrentNode->key.key_base,key.key_base,key.key_len) == 0){
                pthread_mutex_lock(&self->fields_lock);
                self->num_readers--;
                if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
                 }
                pthread_mutex_unlock(&self->fields_lock);
                return CurrentNode->val;
        }
    else{
        int tempindex = index;
        CurrentNode++;
        index++;
        if(index > self->capacity){
            index = 0;
            CurrentNode = ArrayOfNodes;
        }
        while(index != tempindex){
            if(CurrentNode->key.key_len == key.key_len && CurrentNode->tombstone != true && memcmp(CurrentNode->key.key_base,key.key_base,key.key_len) == 0){
                pthread_mutex_lock(&self->fields_lock);
                self->num_readers--;
                if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
                 }
                pthread_mutex_unlock(&self->fields_lock);
                    return CurrentNode->val;
            }
            else if(CurrentNode->key.key_base == NULL){
                pthread_mutex_lock(&self->fields_lock);
                self->num_readers--;
                if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
                 }
                pthread_mutex_unlock(&self->fields_lock);
                return MAP_VAL(NULL, 0);
            }
            else{
                CurrentNode++;
                index++;
            }
            if(index > self->capacity){
            index = 0;
            CurrentNode = ArrayOfNodes;
            }
        }
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0){
        pthread_mutex_unlock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);

    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    pthread_mutex_lock(&self->write_lock);
    map_node_t *ArrayOfNodes = self->nodes;
    int index = get_index(self,key);
    map_node_t *CurrentNode = ArrayOfNodes + index;
    if(CurrentNode->key.key_len == key.key_len && CurrentNode->tombstone != true){
        if(memcmp(CurrentNode->key.key_base,key.key_base,key.key_len) == 0){
            CurrentNode->tombstone = true;
            self->size--;
            pthread_mutex_unlock(&self->write_lock);
            return *CurrentNode;
        }
    }
    else{
        int tempindex = index;
        CurrentNode++;
        index++;
        if(index > self->capacity){
            index = 0;
            CurrentNode = ArrayOfNodes;
        }
        while(index != tempindex){
            if(CurrentNode->key.key_len == key.key_len && CurrentNode->tombstone != true){
                if(memcmp(CurrentNode->key.key_base,key.key_base,key.key_len) == 0){
                    CurrentNode->tombstone = true;
                    pthread_mutex_unlock(&self->write_lock);
                    return *CurrentNode;
            }
            }
            else if(CurrentNode->key.key_base == NULL){
                pthread_mutex_unlock(&self->write_lock);
                return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
            }
            else{
                CurrentNode++;
                index++;
            }
            if(index > self->capacity){
            index = 0;
            CurrentNode = ArrayOfNodes;
            }
        }
    }
    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {
    if(self == NULL) {
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->write_lock);
    map_node_t *ArrayOfNodes = self->nodes;
    destructor_f d_f = self->destroy_function;
    int index = 0;
    while(index < self->capacity){
        map_node_t *CurrentNode = ArrayOfNodes + index;
        if(CurrentNode->tombstone != true){
        CurrentNode->tombstone = true;
        d_f(CurrentNode->key,CurrentNode->val);
        }
        index++;
    }
    self->size = 0;
    pthread_mutex_unlock(&self->write_lock);
	return true;
}

bool invalidate_map(hashmap_t *self) {
    if(self == NULL) {
        errno = EINVAL;
        return false;
    }
    clear_map(self);
    pthread_mutex_lock(&self->write_lock);
    self->invalid = true;
    free(self->nodes);
    pthread_mutex_unlock(&self->write_lock);
    return true;
}
