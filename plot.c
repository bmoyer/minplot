#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "array.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define BATCH_SIZE 1000000

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_reading_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;

int data[BATCH_SIZE];
int stdin_buffer_length = 0;

WINDOW* mainwin = NULL;

// Normalizes data with respect to x and y
void summarize_bargraph(array* samples, int* result, int y, int x) {
    /*
    for(int i = 0; (i < samples->size) && (i < x); i++) {
        result[i] = samples->array[i]->val;
    }
    */

    int slice_size = MAX(samples->size / x, 1);
    int cur_slice = 0;
    for(int i = 0; i < samples->size && cur_slice < x; i+=slice_size) {
        int avg = 0;
        for(int j = i; j < (i+slice_size); j++) {
            avg += samples->array[j]->val;
        }
        avg /= slice_size;
        result[cur_slice++] = avg;
    }
/*
    double slice_size = (double) (samples->size) / (double) x;
    //printf("slice size: %f, samples->size: %d -- %d\n", slice_size, samples->size, 0 < (double) (samples->size) );
    for(double i = 0; i < (double) (samples->size); i+=slice_size) {
        double start = i;
        double end = start + slice_size;

       // double start_int, start_frac;
        //start_frac = modf(start, &start_int);
        double start_frac = ceil(start) - start;
        double end_frac = 
    }
    //printf("RETURN\n");
    */
}

void paint_bargraph(array* samples) {
    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);

    int* result = malloc(num_cols*sizeof(int));
    memset(result, 0, num_cols*sizeof(int));
    summarize_bargraph(samples, result, num_rows, num_cols);
    for(int i = 0; i < 40; i++) {
        //result[i] = i;
        //result[i+40] = 40 - i;
    }
    for(int i = 0; i < num_cols; i++)  {
        int height = result[i];
        //printf("(x, y): (%d, %d)\n", i, height);
        for(int j = 0; j < height; j++) {
            mvwaddch(mainwin, num_rows - j - 2, i + 1, ACS_VLINE);
            //mvwaddch(mainwin, num_rows - j - 2, 80, ACS_VLINE);
        }
        //int i = result[0];
        //mvwaddch(mainwin, 10, 10, ACS_VLINE);
    }
    
    //mvwprintw(mainwin, 20, 10, "%s", "THIS IS A TEST");

    for(int i = 0; i < num_cols; i++) {
        if(i < samples->size) {
            for(int j = 0; j < samples->array[i]->val; j++) {
                //mvwaddch(mainwin, num_rows - j - 2, i + 1, ACS_VLINE);
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

    time_t ltime = time(NULL);
    mvprintw(0, 1, " Data size: %d - %s", samples->size, asctime(localtime(&ltime)));
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

