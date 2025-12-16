#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX 100 //Maximum number of disk requests

int requests[MAX], n = 0, head, disk_size;
char direction[10];
sem_t sem;

int cmpfunc(const void *a, const void *b) {
    return ((int)a - (int)b);  //qsort() to sort integers in ascending order
}

void plot_sequence(int sequence[], int count) {
    printf("\nDisk Head Movement Visualization:\n");

    for (int i = 0; i <= disk_size; i += 10)
        printf("%4d", i);
    printf("\n");

    for (int i = 0; i <= disk_size; i += 10)
        printf("----");
    printf("\n");

    for (int i = 0; i < count; i++) {
        int pos = sequence[i];
        for (int j = 0; j <= disk_size; j += 10)
            printf((pos >= j && pos < j + 10) ? "  ● " : "    ");
        printf("\n");

        for (int j = 0; j <= disk_size; j += 10)
            printf((pos >= j && pos < j + 10) ? " [%d]" : "    ", i + 1);
        printf("\n\n");
    }
}

void* fcfs(void* arg) {
    sem_wait(&sem);
    int total = 0;
    int sequence[n + 1];
    int local_head = head;
    sequence[0] = local_head;

    for (int i = 0; i < n; i++) {
        total += abs(requests[i] - local_head);
        local_head = requests[i];
        sequence[i + 1] = local_head;
    }

    printf("\nFCFS Algorithm:\n");
    plot_sequence(sequence, n + 1);
    printf("Total Head Movement: %d\n", total);
    sem_post(&sem);
    return NULL;
}

void* sstf(void* arg) {
    sem_wait(&sem);
    int visited[n], total = 0;
    int sequence[n + 1];
    int local_head = head;
    sequence[0] = local_head;
    for (int i = 0; i < n; i++) visited[i] = 0;

    for (int i = 0; i < n; i++) {
        int min = 1e9, index = -1;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && abs(requests[j] - local_head) < min) {
                min = abs(requests[j] - local_head);
                index = j;
            }
        }
        total += min;
        local_head = requests[index];
        visited[index] = 1;
        sequence[i + 1] = local_head;
    }

    printf("\nSSTF Algorithm:\n");
    plot_sequence(sequence, n + 1);
    printf("Total Head Movement: %d\n", total);
    sem_post(&sem);
    return NULL;
}

void* scan(void* arg) {
    sem_wait(&sem);
    int left[n], right[n], l = 0, r = 0, total = 0;
    int sequence[n + 3], idx = 0;
    int local_head = head;
    sequence[idx++] = local_head;

    for (int i = 0; i < n; i++) {
        if (requests[i] < local_head) left[l++] = requests[i];
        else right[r++] = requests[i];
    }

    qsort(left, l, sizeof(int), cmpfunc);
    qsort(right, r, sizeof(int), cmpfunc);

    if (strcmp(direction, "left") == 0) {
        for (int i = l - 1; i >= 0; i--) {
            total += abs(local_head - left[i]);
            local_head = left[i];
            sequence[idx++] = local_head;
        }
        total += local_head;
        local_head = 0;
        sequence[idx++] = local_head;
        for (int i = 0; i < r; i++) {
            total += abs(local_head - right[i]);
            local_head = right[i];
            sequence[idx++] = local_head;
        }
    } else {
        for (int i = 0; i < r; i++) {
            total += abs(local_head - right[i]);
            local_head = right[i];
            sequence[idx++] = local_head;
        }
        total += abs(local_head - (disk_size - 1));
        local_head = disk_size - 1;
        sequence[idx++] = local_head;
        total += local_head;
        local_head = 0;
        sequence[idx++] = local_head;
        for (int i = 0; i < l; i++) {
            total += abs(local_head - left[i]);
            local_head = left[i];
            sequence[idx++] = local_head;
        }
    }

    printf("\nSCAN Algorithm (%s):\n", direction);
    plot_sequence(sequence, idx);
    printf("Total Head Movement: %d\n", total);
    sem_post(&sem);
    return NULL;
}

void* cscan(void* arg) {
    sem_wait(&sem);
    int left[n], right[n], l = 0, r = 0, total = 0;
    int sequence[n + 4], idx = 0;
    int local_head = head;
    sequence[idx++] = local_head;

    for (int i = 0; i < n; i++) {
        if (requests[i] < local_head) left[l++] = requests[i];
        else right[r++] = requests[i];
    }

    qsort(left, l, sizeof(int), cmpfunc);
    qsort(right, r, sizeof(int), cmpfunc);

    for (int i = 0; i < r; i++) {
        total += abs(local_head - right[i]);
        local_head = right[i];
        sequence[idx++] = local_head;
    }

    total += abs(local_head - (disk_size - 1));
    local_head = disk_size - 1;
    sequence[idx++] = local_head;

    total += disk_size - 1;
    local_head = 0;
    sequence[idx++] = local_head;

    for (int i = 0; i < l; i++) {
        total += abs(local_head - left[i]);
        local_head = left[i];
        sequence[idx++] = local_head;
    }

    printf("\nCSCAN Algorithm:\n");
    plot_sequence(sequence, idx);
    printf("Total Head Movement: %d\n", total);
    sem_post(&sem);
    return NULL;
}

void* look(void* arg) {
    sem_wait(&sem);
    int left[n], right[n], l = 0, r = 0, total = 0;
    int sequence[n + 2], idx = 0;
    int local_head = head;
    sequence[idx++] = local_head;

    for (int i = 0; i < n; i++) {
        if (requests[i] < local_head) left[l++] = requests[i];
        else right[r++] = requests[i];
    }

    qsort(left, l, sizeof(int), cmpfunc);
    qsort(right, r, sizeof(int), cmpfunc);

    if (strcmp(direction, "left") == 0) {
        for (int i = l - 1; i >= 0; i--) {
            total += abs(local_head - left[i]);
            local_head = left[i];
            sequence[idx++] = local_head;
        }
        for (int i = 0; i < r; i++) {
            total += abs(local_head - right[i]);
            local_head = right[i];
            sequence[idx++] = local_head;
        }
    } else {
        for (int i = 0; i < r; i++) {
            total += abs(local_head - right[i]);
            local_head = right[i];
            sequence[idx++] = local_head;
        }
        for (int i = l - 1; i >= 0; i--) {
            total += abs(local_head - left[i]);
            local_head = left[i];
            sequence[idx++] = local_head;
        }
    }

    printf("\nLOOK Algorithm (%s):\n", direction);
    plot_sequence(sequence, idx);
    printf("Total Head Movement: %d\n", total);
    sem_post(&sem);
    return NULL;
}

void* clook(void* arg) {
    sem_wait(&sem);
    int left[n], right[n], l = 0, r = 0, total = 0;
    int sequence[n + 2], idx = 0;
    int local_head = head;
    sequence[idx++] = local_head;

    for (int i = 0; i < n; i++) {
        if (requests[i] < local_head) left[l++] = requests[i];
        else right[r++] = requests[i];
    }

    qsort(left, l, sizeof(int), cmpfunc);
    qsort(right, r, sizeof(int), cmpfunc);

    for (int i = 0; i < r; i++) {
        total += abs(local_head - right[i]);
        local_head = right[i];
        sequence[idx++] = local_head;
    }

    if (l > 0) {
        total += abs(local_head - left[0]);
        local_head = left[0];
        sequence[idx++] = local_head;

        for (int i = 1; i < l; i++) {
            total += abs(local_head - left[i]);
            local_head = left[i];
            sequence[idx++] = local_head;
        }
    }

    printf("\nCLOOK Algorithm:\n");
    plot_sequence(sequence, idx);
    printf("Total Head Movement: %d\n", total);
    sem_post(&sem);
    return NULL;
}

int main() {
    printf("Enter disk size: ");
    scanf("%d", &disk_size);

    printf("Enter number of requests: ");
    scanf("%d", &n);

    printf("Enter requests sequence:\n");
    for (int i = 0; i < n; i++)
        scanf("%d", &requests[i]);

    printf("Enter initial head position: ");
    scanf("%d", &head);

    printf("Enter initial direction (left/right): ");
    scanf("%s", direction);

    sem_init(&sem, 0, 1);

    pthread_t t1, t2, t3, t4, t5, t6;

    pthread_create(&t1, NULL, fcfs, NULL);
    pthread_join(t1, NULL);

    pthread_create(&t2, NULL, sstf, NULL);
    pthread_join(t2, NULL);

    pthread_create(&t3, NULL, scan, NULL);
    pthread_join(t3, NULL);

    pthread_create(&t4, NULL, cscan, NULL);
    pthread_join(t4, NULL);

    pthread_create(&t5, NULL, look, NULL);
    pthread_join(t5, NULL);

    pthread_create(&t6, NULL, clook, NULL);
    pthread_join(t6, NULL);

    sem_destroy(&sem);
    return 0;
}

