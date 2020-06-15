#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include "array.h"

#define BATCH_SIZE 1000000

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_reading_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;

int data[BATCH_SIZE];
int stdin_buffer_length = 0;

WINDOW* mainwin = NULL;

// Normalizes data with respect to x and y
void summarize_bargraph(array* samples, int* result, int y, int x) {
    int num_slices = 0;
    if(samples->size < x){
        for(int i = 0; i < samples->size; i++) {
            result[i] = samples->array[i]->val;
            num_slices++;
        }
    }
    else {
        int cur_slice = 0;
        double slice_size = (double) (samples->size) / (double) x;
        for(double i = 0; i < (double) (samples->size) - slice_size; i+=slice_size) {
            double start = i;
            double end = start + slice_size;

            double start_frac, end_frac;
            if(ceil(start) == start) {
                start_frac = 1.0;
            }
            else {
                start_frac = ceil(start) - start;
            }

            if(ceil(end) == end) {
                end_frac = 1.0;
            }
            else {
                end_frac = 1.0 - (ceil(end) - end);
            }
            
            double total = 0.0;
            for(int i = (int)start; i <= (int)end; i++) {
                if(i == (int)start) {
                    total += start_frac * (double) samples->array[i]->val;
                }
                else if(i == (int)(end)) {
                    total += end_frac * (double)samples->array[i]->val;
                }
                else {
                    total += (double)samples->array[i]->val;
                }
            }
            total = round(total / slice_size);
            result[cur_slice++] = total;
            num_slices++;
            //fprintf(stderr, "start_frac %f end_frac %f (%f -> %f), %f\r\n", start_frac, end_frac, start, end, total);
        }
    }

    // Now scale data in Y
    int minVal = INT_MAX;
    int maxVal = INT_MIN;

    for(int i = 0; i < num_slices; i++) {
        minVal = MIN(minVal, result[i]);
        maxVal = MAX(maxVal, result[i]);
    }
    /*
    double scaleY = floor(maxVal - 0) / y;

    for(int i = 0; i < num_slices; i++) {
        result[i] = round(result[i] / scaleY);
    }
    */
    double scaleY = (maxVal - 0) / y;

    for(int i = 0; i < num_slices; i++) {
        result[i] = floor(result[i] / scaleY);
    }
}

void paint_bargraph(array* samples) {
    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);

    int* result = malloc(num_cols*sizeof(int)*1);
    memset(result, 0, num_cols*sizeof(int));
    summarize_bargraph(samples, result, num_rows, num_cols);

    for(int i = 0; i < num_cols; i++)  {
        int height = result[i];
        for(int j = 0; j < height; j++) {
            mvwaddch(mainwin, num_rows - j - 2, i + 1, ACS_VLINE);
        }
    }
    free(result);
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

    erase();
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
        usleep(50000);
    }

    endwin();
    delwin(mainwin);

    pthread_join(thread_id, NULL);
    printf("Main thread exiting\n");

    return 0;
}

