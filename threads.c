#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t ops_lock   = PTHREAD_MUTEX_INITIALIZER; 
int shared_ops = 0;  

void catat_selesai(void) {
    pthread_mutex_lock(&ops_lock);      
    shared_ops++;
    pthread_mutex_unlock(&ops_lock);    
}

void *hitung_faktorial(void *arg) {
    int n = *(int *)arg;
    unsigned long long hasil = 1;
    for (int i = 2; i <= n; i++) hasil *= i;

    pthread_mutex_lock(&print_lock);
    printf("[Thread 1] Faktorial %d! = %llu\n", n, hasil);
    pthread_mutex_unlock(&print_lock);
    catat_selesai();
    return NULL;
}

void *tampil_fibonacci(void *arg) {
    int n = *(int *)arg;
    unsigned long long a = 0, b = 1;

    pthread_mutex_lock(&print_lock);
    printf("[Thread 2] Fibonacci (%d suku): ", n);
    for (int i = 0; i < n; i++) {
        printf("%llu ", a);
        unsigned long long t = a + b; a = b; b = t;
    }
    printf("\n");
    pthread_mutex_unlock(&print_lock);
    catat_selesai();
    return NULL;
}

void *baca_file(void *arg) {
    const char *nama = (const char *)arg;
    FILE *f = fopen(nama, "r");
    pthread_mutex_lock(&print_lock);
    if (!f) {
        printf("[Thread 3] Gagal membuka file %s\n", nama);
    } else {
        char baris[256];
        printf("[Thread 3] Isi file %s:\n", nama);
        while (fgets(baris, sizeof baris, f)) printf("           | %s", baris);
        fclose(f);
    }
    pthread_mutex_unlock(&print_lock);
    catat_selesai();
    return NULL;
}

#define JUMLAH_THREAD 4
#define ITERASI 20000
volatile long counter_tanpa_lock = 0; 
long counter_dengan_lock = 0;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void *tambah_tanpa_lock(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERASI; i++) {
        long tmp = counter_tanpa_lock;   
        sched_yield();                   
        counter_tanpa_lock = tmp + 1;    
    }
    return NULL;
}
void *tambah_dengan_lock(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERASI; i++) {
        pthread_mutex_lock(&counter_lock);
        long tmp = counter_dengan_lock;  
        sched_yield();
        counter_dengan_lock = tmp + 1;
        pthread_mutex_unlock(&counter_lock);
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2, t3;
    int n_fakt = 10, n_fib = 15;

    printf("=== Bagian A: 3 thread dengan pekerjaan berbeda ===\n");
    pthread_create(&t1, NULL, hitung_faktorial, &n_fakt);
    pthread_create(&t2, NULL, tampil_fibonacci, &n_fib);
    pthread_create(&t3, NULL, baca_file, (void *)"data.txt");
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    printf("Jumlah thread selesai (shared_ops) = %d (harapan: 3)\n\n", shared_ops);

    printf("=== Bagian B: Demonstrasi race condition ===\n");
    pthread_t th[JUMLAH_THREAD];
    for (int i = 0; i < JUMLAH_THREAD; i++) pthread_create(&th[i], NULL, tambah_tanpa_lock, NULL);
    for (int i = 0; i < JUMLAH_THREAD; i++) pthread_join(th[i], NULL);
    printf("Tanpa mutex : %ld (harapan: %d)\n", counter_tanpa_lock, JUMLAH_THREAD * ITERASI);

    for (int i = 0; i < JUMLAH_THREAD; i++) pthread_create(&th[i], NULL, tambah_dengan_lock, NULL);
    for (int i = 0; i < JUMLAH_THREAD; i++) pthread_join(th[i], NULL);
    printf("Dengan mutex: %ld (harapan: %d)\n", counter_dengan_lock, JUMLAH_THREAD * ITERASI);
    return 0;
}