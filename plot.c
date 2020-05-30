#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#include "array.h"

#define BATCH_SIZE 1000000

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_reading_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;

int data[BATCH_SIZE];
int stdin_buffer_length = 0;

WINDOW* mainwin = NULL;

// Normalizes data with respect to x and y
void summarize_bargraph(array* samples, int* result, int y, int x) {
    result = malloc(x * sizeof(int));
    for(int i = 0; i < samples->size; i++) {
    }
}

void paint_bargraph(array* samples) {
    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);

    for(int i = 0; i < num_cols; i++) {
        if(i < samples->size) {
            for(int j = 0; j < samples->array[i]->val; j++) {
                mvwaddch(mainwin, num_rows - j - 2, i + 1, ACS_VLINE);
            }
        }
    }
}

void paint(array* samples) {
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

    paint_bargraph(samples);

    mvprintw(0, 1, " Data size: %d ", samples->size);
    refresh();
}

void* read_stdin_thread(void* vargp) {
    char* line = NULL;
    size_t size;
    while(getline(&line, &size, stdin) != -1) {
        pthread_mutex_lock(&buffer_mutex);
        if(stdin_buffer_length == BATCH_SIZE) {
            pthread_cond_wait(&buffer_empty, &buffer_mutex);
        }
        data[stdin_buffer_length++] = atoi(line);
        pthread_mutex_unlock(&buffer_mutex);
    }

    return NULL;
}

int main() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_stdin_thread, NULL);

    array* samples = malloc(sizeof(array));
    init_array(samples, 10000);
    while(true) {
        // Copy data from shared buffer into local buffer, reset size
        pthread_mutex_lock(&buffer_mutex);

        for(int i = 0; i < stdin_buffer_length; i++) {
            data_point* dp = malloc(sizeof(data_point));
            dp->val = data[i];
            insert_array(samples, dp);
        }
        stdin_buffer_length = 0;
        pthread_cond_signal(&buffer_empty);
        pthread_mutex_unlock(&buffer_mutex);

        // Update the UI
        paint(samples);

        // Sleep for remainder of time slice
        sleep(1);
    }

    endwin();
    delwin(mainwin);

    pthread_join(thread_id, NULL);
    printf("Main thread exiting\n");

    return 0;
}

