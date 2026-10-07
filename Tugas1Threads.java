import java.io.*;
import java.util.concurrent.atomic.*;
import java.util.concurrent.locks.*;

public class Tugas1Threads {
    static final Object printLock = new Object();
    static int sharedOps = 0;
    static synchronized void catatSelesai() { sharedOps++; }   

    static int counterTanpaLock = 0;
    static int counterDenganLock = 0;
    static final ReentrantLock lock = new ReentrantLock();

    public static void main(String[] args) throws Exception {
        System.out.println("=== Bagian A: 3 thread dengan pekerjaan berbeda ===");
        Thread t1 = new Thread(() -> {
            long hasil = 1;
            for (int i = 2; i <= 10; i++) hasil *= i;
            synchronized (printLock) { System.out.println("[Thread 1] Faktorial 10! = " + hasil); }
            catatSelesai();
        });
        Thread t2 = new Thread(() -> {
            StringBuilder sb = new StringBuilder("[Thread 2] Fibonacci (15 suku): ");
            long a = 0, b = 1;
            for (int i = 0; i < 15; i++) { sb.append(a).append(' '); long t = a + b; a = b; b = t; }
            synchronized (printLock) { System.out.println(sb); }
            catatSelesai();
        });
        Thread t3 = new Thread(() -> {
            synchronized (printLock) {
                try (BufferedReader br = new BufferedReader(new FileReader("data.txt"))) {
                    System.out.println("[Thread 3] Isi file data.txt:");
                    String line;
                    while ((line = br.readLine()) != null) System.out.println("           | " + line);
                } catch (IOException e) { System.out.println("[Thread 3] Gagal: " + e.getMessage()); }
            }
            catatSelesai();
        });
        t1.start(); t2.start(); t3.start();
        t1.join(); t2.join(); t3.join();
        System.out.println("Jumlah thread selesai (sharedOps) = " + sharedOps + " (harapan: 3)\n");

        System.out.println("=== Bagian B: Demonstrasi race condition ===");
        final int N = 4, ITER = 1_000_000;
        Thread[] th = new Thread[N];
        for (int i = 0; i < N; i++) { th[i] = new Thread(() -> { for (int j = 0; j < ITER; j++) counterTanpaLock++; }); th[i].start(); }
        for (Thread t : th) t.join();
        System.out.println("Tanpa lock  : " + counterTanpaLock + " (harapan: " + N * ITER + ")");

        for (int i = 0; i < N; i++) {
            th[i] = new Thread(() -> {
                for (int j = 0; j < ITER; j++) { lock.lock(); try { counterDenganLock++; } finally { lock.unlock(); } }
            });
            th[i].start();
        }
        for (Thread t : th) t.join();
        System.out.println("Dengan lock : " + counterDenganLock + " (harapan: " + N * ITER + ")");
    }
}