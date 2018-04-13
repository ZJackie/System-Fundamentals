#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "csapp.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>


void *service_request();
void map_free_function(map_key_t key, map_val_t val);
queue_t *request_queue;
hashmap_t *global_map;

int main(int argc, char *argv[]) {
    int num_of_workers;
    char* port_number;
    int max_entries;
    int listenfd, connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    signal(SIGPIPE, SIG_IGN);
    //Help Check
    for(int i = 1; i < argc; i++){
        if(strcmp( *(argv+i), "-h") == 0){
            printf("Help\n");
            return EXIT_SUCCESS;
        }
    }

    if(argc < 4){
        printf("Invald number of args\n");
    }
    else{
        num_of_workers = atoi(*(argv + 1));
        port_number = *(argv + 2);
        max_entries = atoi(*(argv + 3));
        request_queue = create_queue();
        global_map = create_map(max_entries, jenkins_one_at_a_time_hash, map_free_function);

        pthread_t thread_ids;
        for(int index = 0; index < num_of_workers; index++) {
            if(pthread_create(&thread_ids, NULL, service_request, NULL) != 0)
            exit(EXIT_FAILURE);
        }

        listenfd = open_listenfd(port_number);
        while(1){
            clientlen = sizeof(struct sockaddr_storage);
            connfdp = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
            enqueue(request_queue, &connfdp);
        }
        return EXIT_SUCCESS;
    }
}

void *service_request() {
            pthread_detach(pthread_self());
            while(1){
            int connfd = *((int*) dequeue(request_queue));
            request_header_t request_header;
            response_header_t response_header = {OK, sizeof(response_header)};
            Rio_readn(connfd, &request_header, sizeof(request_header));
            if(request_header.request_code == PUT){
                if(request_header.key_size < MIN_KEY_SIZE || request_header.key_size > MAX_KEY_SIZE){
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connfd, &response_header, sizeof(response_header));
                }
                else if(request_header.value_size < MIN_VALUE_SIZE || request_header.value_size > MAX_VALUE_SIZE){
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connfd, &response_header, sizeof(response_header));
                }
                else{
                char* key = Calloc(1,request_header.key_size);
                char* value = Calloc(1,request_header.value_size);

                Rio_readn(connfd, key, request_header.key_size);
                Rio_readn(connfd, value, request_header.value_size);
                map_key_t input_key = {key, request_header.key_size};
                map_val_t input_val = {value, request_header.value_size};
                if(global_map->size == global_map->capacity){
                    put(global_map,input_key,input_val,1);
                }
                else{
                    put(global_map,input_key,input_val,0);
                }
                Rio_writen(connfd, &response_header, sizeof(response_header));
            }
            }
            else if(request_header.request_code == GET){
                if(request_header.key_size < MIN_KEY_SIZE || request_header.key_size > MAX_KEY_SIZE){
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connfd, &response_header, sizeof(response_header));
                }
                else{
                char* key = Calloc(1,request_header.key_size);
                Rio_readn(connfd, key, request_header.key_size);
                map_key_t input_key = {key, request_header.key_size};
                map_val_t returnvalue = get(global_map,input_key);
                if(returnvalue.val_base == NULL){
                response_header.response_code = NOT_FOUND;
                response_header.value_size = 0;
                Rio_writen(connfd, &response_header, sizeof(response_header));
                }
                else{
                Rio_writen(connfd, &response_header, sizeof(response_header));
                Rio_writen(connfd, returnvalue.val_base, returnvalue.val_len);
            }
            }
            }
            else if(request_header.request_code == EVICT){
                if(request_header.key_size < MIN_KEY_SIZE || request_header.key_size > MAX_KEY_SIZE){
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connfd, &response_header, sizeof(response_header));
                }
                char* key = Calloc(1,request_header.key_size);
                Rio_readn(connfd, key, request_header.key_size);
                map_key_t input_key = {key, request_header.key_size};
                delete(global_map,input_key);
                Rio_writen(connfd, &response_header, sizeof(response_header));
            }
            else if(request_header.request_code == CLEAR){
                 clear_map(global_map);
                 Rio_writen(connfd, &response_header, sizeof(response_header));
            }
            else{
                response_header.response_code = UNSUPPORTED;
                response_header.value_size = 0;
                Rio_writen(connfd, &response_header, sizeof(response_header));
            }
                close(connfd);
            }
            return NULL;
}

void map_free_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}