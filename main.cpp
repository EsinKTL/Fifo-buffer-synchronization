#include <iostream>
#include <thread>
#include <unistd.h>
#include "semaphore.h"

// --- BUFFER ---
char buffer[9];
int in = 0;
int out = 0;
int count = 0; // Current number of elements in the buffer

// --- STATE VARIABLES ---
int turn = 0;         // 0: Producer A's turn, 1: Producer B's turn
int a_produced = 0;   // Consecutive 'A's produced in the current turn
int b_produced = 0;   // Consecutive 'B's produced in the current turn

// --- PASSING THE BATON SEMAPHORES ---
Semaphore mutex(1);   // Main lock protecting the critical section
Semaphore semA(0);    // Private semaphore to block Producer A
Semaphore semB(0);    // Private semaphore to block Producer B
Semaphore semCons(0); // Private semaphore to block Consumers

// Counters for waiting threads
int waitingA = 0;
int waitingB = 0;
int waitingCons = 0;

/**
 * The "Passing the Baton" signal function.
 * Instead of simply releasing the mutex, it checks if any waiting thread's 
 * condition has been met. If so, it transfers the lock directly to that thread.
 */
void signal_next() {
    if (count < 9 && turn == 0 && waitingA > 0) {
        semA.v(); // Wake up A (Directly pass the baton/mutex)
    } else if (count < 9 && turn == 1 && waitingB > 0) {
        semB.v(); // Wake up B (Directly pass the baton/mutex)
    } else if (count > 3 && waitingCons > 0) {
        semCons.v(); // Wake up Consumer (Directly pass the baton/mutex)
    } else {
        mutex.v(); // No one can proceed, release the mutex back to the system
    }
}

// --- PRODUCER A THREAD ---
void producerA(int id) {
    while (true) {
        mutex.p(); // Enter critical section
        
        // CONDITION: Wait if buffer is full OR it's not A's turn
        if (!(count < 9 && turn == 0)) {
            waitingA++;
            mutex.v(); // Release mutex before going to sleep
            semA.p();  // Sleep here. When awakened, the lock is already held!
            waitingA--;
        }

        // --- Critical Section: Produce 'A' ---
        buffer[in] = 'A';
        std::cout << "Producer A[" << id << "] produced: A (Buffer items: " << count + 1 << "/9)\n";
        in = (in + 1) % 9;
        count++;
        a_produced++;

        // Rule: If A has produced 3 consecutively, hand over the turn to B
        if (a_produced == 3) {
            a_produced = 0;
            turn = 1;
        }

        signal_next(); // Pass the baton
        usleep(100000); // Slight delay for readable console output
    }
}

// --- PRODUCER B THREAD ---
void producerB(int id) {
    while (true) {
        mutex.p(); // Enter critical section
        
        // CONDITION: Wait if buffer is full OR it's not B's turn
        if (!(count < 9 && turn == 1)) {
            waitingB++;
            mutex.v(); // Release mutex before going to sleep
            semB.p();  // Sleep here.
            waitingB--;
        }

        // --- Critical Section: Produce 'B' ---
        buffer[in] = 'B';
        std::cout << "Producer B[" << id << "] produced: B (Buffer items: " << count + 1 << "/9)\n";
        in = (in + 1) % 9;
        count++;
        b_produced++;

        // Rule: If B has produced 2 consecutively, hand over the turn to A
        if (b_produced == 2) {
            b_produced = 0;
            turn = 0;
        }

        signal_next(); // Pass the baton
        usleep(100000);
    }
}

// --- CONSUMER THREAD ---
void consumer(int id) {
    while (true) {
        mutex.p(); // Enter critical section
        
        // CONDITION: Wait if there are NOT strictly more than 3 elements
        if (!(count > 3)) {
            waitingCons++;
            mutex.v(); // Release mutex before going to sleep
            semCons.p(); // Sleep here.
            waitingCons--;
        }

        // --- Critical Section: Consume ---
        char item = buffer[out];
        std::cout << "\t\tConsumer [" << id << "] read: " << item << " (Remaining: " << count - 1 << "/9)\n";
        out = (out + 1) % 9;
        count--;

        signal_next(); // Pass the baton
        usleep(150000);
    }
}

int main() {
    std::cout << " 9-Element FIFO Buffer Synchronization Test\n";
    std::cout << "Rules: Initial turn A. A produces 3, B produces 2. Read if count > 3.\n\n";

    // Creating threads as per assignment requirements
    std::thread pA(producerA, 1);
    std::thread pB(producerB, 1);
    
    std::thread c1(consumer, 1);
    std::thread c2(consumer, 2);

    // Join threads (this will run indefinitely)
    pA.join();
    pB.join();
    c1.join();
    c2.join();

    return 0;
}