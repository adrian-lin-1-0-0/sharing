#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define PORT 8080
#define MAX_EVENTS 1024
#define MAX_THREADS 4
#define BUF_SIZE 1024

typedef struct {
    int fd;
} task_t;

typedef struct {
    task_t tasks[1024];
    int front, rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} task_queue_t;

task_queue_t queue;

void task_queue_init(task_queue_t *q) {
    q->front = q->rear = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void task_queue_push(task_queue_t *q, task_t task) {
    pthread_mutex_lock(&q->mutex);
    q->tasks[q->rear++] = task;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

task_t task_queue_pop(task_queue_t *q) {
    pthread_mutex_lock(&q->mutex);
    while (q->front == q->rear)
        pthread_cond_wait(&q->cond, &q->mutex);
    task_t task = q->tasks[q->front++];
    pthread_mutex_unlock(&q->mutex);
    return task;
}

void *worker_thread(void *arg) {
    while (1) {
        task_t task = task_queue_pop(&queue);
        char buf[BUF_SIZE];
        int n = read(task.fd, buf, sizeof(buf));
        if (n > 0) {
            buf[n] = '\0';
            printf("Received from client: %s\n", buf);
            write(task.fd, buf, n); // Echo back
        } else {
            close(task.fd);
        }
    }
    return NULL;
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);
    set_nonblocking(listen_fd);
    // Red black tree for epoll
    int epfd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    
    // Level-triggered epoll
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    task_queue_init(&queue);
    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++)
        pthread_create(&threads[i], NULL, worker_thread, NULL);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == listen_fd) {
                int client_fd = accept(listen_fd, NULL, NULL);
                set_nonblocking(client_fd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
            } else {
                task_t task;
                task.fd = events[i].data.fd;
                task_queue_push(&queue, task);
            }
        }
    }

    close(listen_fd);
    return 0;
}
