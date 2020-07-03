#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <signal.h>

#include "array.h"

#define BATCH_SIZE 1000000
#define FRAME_INTERVAL_MS 50

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef void (*sighandler_t)(int);

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_reading_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;

int data[BATCH_SIZE];
int stdin_buffer_length = 0;

sighandler_t previous_handler;

WINDOW* mainwin = NULL;

// Normalizes data with respect to x and y
void summarize_bargraph(array* samples, int* result, double* scaleY, int y, int x) {
    int minVal = INT_MAX;
    int maxVal = INT_MIN;
    int num_slices = 0;
    if(samples->size < x){
        for(int i = 0; i < samples->size; i++) {
            result[i] = samples->array[i]->val;
            minVal = MIN(minVal, samples->array[i]->val);
            maxVal = MAX(maxVal, samples->array[i]->val);
            num_slices++;
        }
    }
    else {
        int cur_slice = 0;
        double slice_size = (double) (samples->size) / (double) x;
        for(double i = 0; i < (double) (samples->size) /*- slice_size*/; i+=slice_size) {
            double start = i;
            double end = MIN(start + slice_size, samples->size - 1);

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
                minVal = MIN(minVal, samples->array[i]->val);
                maxVal = MAX(maxVal, samples->array[i]->val);
            }
            total = round(total / slice_size);
            result[cur_slice++] = total;
            num_slices++;
        }
    }

    *scaleY = ((double)maxVal - 0.0) / (double)y;
    if(maxVal <= y) {
        *scaleY = 1.0;
    }

    for(int i = 0; i < num_slices; i++) {
        result[i] = floor(result[i] / *scaleY);
    }
}

void paint_footer(array* samples) {
    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);

    time_t ltime = time(NULL);
    char footer[100];
    sprintf(footer, "Data size: %d - %s", (int)samples->size, asctime(localtime(&ltime)));
    mvprintw(num_rows-1, MAX(0, num_cols - strlen(footer) - 1), "%s", footer);
}

void paint_bargraph(array* samples) {
    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);

    int data_height = num_rows - 2;
    int data_width = num_cols - 2;

    double* scaleY = malloc(sizeof(double));
    int* result = malloc(data_width*sizeof(int));
    memset(result, 0, data_width*sizeof(int));
    summarize_bargraph(samples, result, scaleY, data_height, data_width);

    for(int i = 0; i < data_width; i++)  {
        int height = result[i];
        for(int j = 0; j < height; j++) {
            mvwaddch(mainwin, data_height - j - 1, i + 1, ACS_VLINE);
        }
    }

    // Draw axis numbers
    double label_points[4] = {1.0, 0.75, 0.5, 0.25};
    for(size_t i = 0; i < sizeof(label_points)/sizeof(label_points[0]); i++) {
        double val = (*scaleY * (double)data_height * label_points[i]);
        int row = data_height - round(label_points[i] * (double) data_height) + 1;
        mvprintw(row, 2, "%.1f", val);
    }

    free(result);
    free(scaleY);
}

void paint_axes(char* title) {
    int num_rows, num_cols;
    getmaxyx(mainwin, num_rows, num_cols);

    for(int y = 2; y < num_rows-2; y++) {
        mvwaddch(mainwin, y, 0, ACS_VLINE);
    }

    mvwaddch(mainwin, 1, 0, ACS_UARROW);
    mvwaddch(mainwin, num_rows-2, 0, ACS_LLCORNER);

    for(int x = 1; x < num_cols-3; x++) {
        mvwaddch(mainwin, num_rows-2, x, ACS_HLINE);
    }
    mvwaddch(mainwin, num_rows-2, num_cols-2, ACS_RARROW);

    // Paint title
    mvprintw(0, (num_cols - strlen(title))/2, "%s", title);
}

void paint(char* title, array* samples) {
    if(mainwin == NULL) {
        // Initialize main window
        if((mainwin = initscr()) == NULL) {
            perror("Failed to init ncurses");
            exit(1);
        }
        noecho();
    }

    erase();

    paint_axes(title);
    paint_bargraph(samples);
    paint_footer(samples);

    curs_set(0);
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

void resizeHandler(int sig) {
    endwin();
}

int main(int argc, char* argv[]) {
    int opt;
    char* title = "";
    while((opt = getopt(argc, argv, "t:")) != -1) {
        switch(opt) {
            case 't':
                //printf("Title: %s\n", optarg);
                title = optarg;
                break;
            }
    }

    //previous_handler = signal(SIGWINCH, resizeHandler);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_stdin_thread, NULL);

    array* samples = malloc(sizeof(array));
    init_array(samples, 10000);
    struct timespec start_time;
    struct timespec end_time;
    while(true) {
        clock_gettime(CLOCK_REALTIME, &start_time);
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
        paint(title, samples);

        clock_gettime(CLOCK_REALTIME, &end_time);

        long start_nsec = start_time.tv_sec * 1.0e9 + start_time.tv_nsec;
        long end_nsec = end_time.tv_sec * 1.0e9 + end_time.tv_nsec;
        long elapsed_ms = round((end_nsec - start_nsec) / 1.0e6);

        if(elapsed_ms < FRAME_INTERVAL_MS) {
            // Sleep for remainder of time slice
            usleep ((FRAME_INTERVAL_MS - elapsed_ms) * 1000L);
        }
    }

    endwin();
    delwin(mainwin);

    pthread_join(thread_id, NULL);

    return 0;
}

