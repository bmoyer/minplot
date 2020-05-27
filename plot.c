#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#define BATCH_SIZE 1000000

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_reading_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

int data[BATCH_SIZE];
int stdin_buffer_length = 0;

typedef struct node {
    int val;
    struct node* next;
} node;

node* createNode(int val) {
    node* n = malloc(sizeof(node));
    n->val = val;
    n->next = NULL;
    return n;
}

node* head = NULL;
node* tail = NULL;

void append(int val) {
    node* new_tail = createNode(val);

    if(tail != NULL) {
        tail->next = new_tail;
    }

    tail = new_tail;

    if(head == NULL) {
        head = tail;
    }
}

void delete_list(node** n) {
    node* pn = *n;
    while(pn != NULL) {
        node* tmp = pn->next;
        free(pn);
        pn = tmp;
    }
    *n = NULL;
}

void print_list(node* n) {
    while(n != NULL) {
        printf("Node val: %d\n", n->val);
        n = n->next;
    }
}

int list_length(node* n) {
    int len = 0;
    while(n != NULL) {
        len++;
        n = n->next;
    }
    return len;
}

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

void draw_plot() {
    printf("Linked list length: %d\n", list_length(head));
    //print_list(head);
}

int main() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_stdin_thread, NULL);

    int mydata[BATCH_SIZE];
    int mybufsize = 0;
    while(true) {
        // memcpy data from buffer into local buffer, reset size
        pthread_mutex_lock(&buffer_mutex);
        memcpy(mydata, data, stdin_buffer_length * sizeof(int));
        mybufsize = stdin_buffer_length;
        stdin_buffer_length = 0;
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&buffer_mutex);

        for(int i = 0; i < mybufsize; i++) {
            append(mydata[i]);
        }

        draw_plot();
        sleep(1);

        // Sleep for remainder of time slice
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

