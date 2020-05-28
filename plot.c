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

WINDOW* mainwin = NULL;

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

void draw_plot(linked_list* n) {
    if(mainwin == NULL) {
        // Initialize main window
        printf("Initializing main window\n");
        if((mainwin = initscr()) == NULL) {
            perror("Failed to init ncurses");
            exit(1);
        }
        noecho();
    }

    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);
    wborder(mainwin, 0, 0, 0, 0, 0, 0, 0, 0);
    //printf("length was: %d\n\r", list_length(n));
    for(int i = 0; i < 10; i++) {
        //mvwaddch(mainwin, 20, i*2, ACS_PI);
        //mvwaddch(mainwin, 21, i*2 + 1, ACS_STERLING);
    }

    mvprintw(num_rows-1, 1, " Data size: %d ", list_length(n));
    refresh();
    //printf("Linked list length: %d\n", list_length(n));
    //print_list(n);
}

int main() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_stdin_thread, NULL);

    int mydata[BATCH_SIZE];
    int data_count = 0;
    linked_list list;

    while(true) {
        // Copy data from shared buffer into local buffer, reset size
        pthread_mutex_lock(&buffer_mutex);
        memcpy(mydata, data, stdin_buffer_length * sizeof(int));
        data_count = stdin_buffer_length;
        stdin_buffer_length = 0;
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&buffer_mutex);

        // Append new data to list
        for(int i = 0; i < data_count; i++) {
            int* pval = malloc(sizeof(int));
            *pval = mydata[i];
            append(&list, (void*)pval);
        }

        // Update the UI
        draw_plot(&list);

        // Sleep for remainder of time slice
        sleep(1);
    }

    endwin();
    delwin(mainwin);

    pthread_join(thread_id, NULL);
    printf("Main thread exiting\n");

    return 0;

    /*
    */
}

