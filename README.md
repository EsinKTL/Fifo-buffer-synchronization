# Multi-Threaded FIFO Buffer Synchronization (Passing the Baton)

This repository contains a high-level synchronization solution for a multi-producer multi-consumer FIFO buffer problem, implemented in C++ using POSIX threads and custom semaphores. The project focuses on solving complex state-dependent synchronization constraints using the **"Passing the Baton"** (Przekazywanie pałeczki) pattern.

## Problem Definition

The task was to manage a **9-element FIFO buffer** with the following strict constraints:

### Production Rules:
- **Producer A:** Produces letter 'A'. It can only start if it's its turn.
- **Producer B:** Produces letter 'B'. It can only produce if Producer A has produced **exactly 3 letters** consecutively.
- **Cycle:** After Producer B produces **2 letters**, the right to produce returns to A.
- **Initial State:** The production right starts with Producer A.

### Consumption Rules:
- **Consumers:** Two consumer threads read from the buffer.
- **Threshold:** Reading (deleting) is only allowed if the buffer contains **strictly more than 3 elements**.

### Synchronization Requirements:
- Use only **P (wait)** and **V (signal)** operations.
- Must be implemented using the `Semaphore` class provided in `semaphore.h`.
- No "Busy Waiting" is allowed; threads must sleep when conditions are not met.

---

## Technical Implementation: "Passing the Baton"

Standard semaphores (counting semaphores) are insufficient for state-dependent conditions like "exactly 3 A's then 2 B's". To ensure safety and liveness, I implemented the **Passing the Baton** pattern:

- **Private Semaphores:** Each thread type (A, B, Consumer) has a private semaphore to sleep on.
- **Baton Transfer:** Instead of simply releasing a mutex, a finishing thread checks the conditions of other waiting threads. If a condition is met, the lock is transferred directly to that thread without releasing it to the general scheduler.
- **Safety:** This prevents "Spurious Wakeups" and ensures that once a thread wakes up, it is guaranteed that its execution condition is still valid.

---

## Project Structure

- `main.cpp`: Core logic including producer/consumer routines and the baton-passing signal function.
- `semaphore.h`: Custom semaphore class provided for the assignment.
- `Makefile`: Automated build script for Linux environments.

---

## How to Run

1. **Prerequisites:** Ensure you are in a Linux environment (or WSL/Ubuntu) with `g++` and `make` installed.
2. **Build:**
   make
3. **Execute:** ./fifo_task
4.**Exit:** Press Ctrl + C to stop the simulation.

## Verification
In the provided logs, you can verify:

**Turn Management:** Production strictly follows the AAA -> BB -> AAA sequence.

**Threshold Logic:** Consumers do not read until the buffer count reaches 4.

**Deadlock Freedom:** The system runs indefinitely without freezing, proving the correctness of the synchronization logic.

***Developed as a laboratory assignment for the Operating Systems / Concurrent Programming course at Warsaw University of Technology (WUT).***
