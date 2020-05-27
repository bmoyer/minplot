#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#include "linked_list.h"

#define BATCH_SIZE 1000000

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_reading_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

int data[BATCH_SIZE];
int stdin_buffer_length = 0;

void* read_stdin_thread(void* vargp) {
    char* line = NULL;
    size_t size;
    while(getline(&line, &size, stdin) != -1) {
        pthread_mutex_lock(&buffer_mutex);
        if(stdin_buffer_length == BATCH_SIZE) {
            pthread_cond_wait(&cond1, &buffer_mutex);
        }
        data[stdin_buffer_length++] = atoi(line);
        pthread_mutex_unlock(&buffer_mutex);
    }

    return NULL;
}

void draw_plot(node* n) {
    printf("Linked list length: %d\n", list_length(n));
    //print_list(n);
}

int main() {
    node* head = NULL;
    node* tail = NULL;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_stdin_thread, NULL);

    int mydata[BATCH_SIZE];
    int mybufsize = 0;
    while(true) {
        // Copy data from shared buffer into local buffer, reset size
        pthread_mutex_lock(&buffer_mutex);
        memcpy(mydata, data, stdin_buffer_length * sizeof(int));
        mybufsize = stdin_buffer_length;
        stdin_buffer_length = 0;
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&buffer_mutex);

        // Append new data to list
        for(int i = 0; i < mybufsize; i++) {
            int* pval = malloc(sizeof(int));
            *pval = mydata[i];
            append(&head, &tail, (void*)pval);
        }

        // Update the UI
        draw_plot(head);

        // Sleep for remainder of time slice
        sleep(1);
    }

    pthread_join(thread_id, NULL);
    printf("Main thread exiting\n");

    return 0;

    /*
    WINDOW* mainwin;
    if((mainwin = initscr()) == NULL) {
        perror("Failed to init ncurses");
        exit(1);
    }

    noecho();

    wborder(mainwin, 0, 0, 0, 0, 0, 0, 0, 0);
    //box(stdscr, 0, 0);
    for(int i = 0; i < 10; i++) {
        mvwaddch(mainwin, 20, i*2, ACS_PI);
        mvwaddch(mainwin, 21, i*2 + 1, ACS_STERLING);
    }

    refresh();
    sleep(1);
    endwin();
    delwin(mainwin);
    */
}

