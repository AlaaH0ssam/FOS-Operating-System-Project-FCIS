# Educational Operating System Project (FOS – OS’25)

This repository contains the implementation of an **educational operating system** developed as part of the **OS’25 course project at Ain Shams University**.  
The project provides a hands-on exploration of core operating system concepts through **kernel-level programming**, modular design, and rigorous testing.

The system is built incrementally, where each module depends on previously implemented components. A strong emphasis is placed on **correctness by design**, safe synchronization, and full compliance with provided testing frameworks.

---

## 📌 Project Overview

The objective of this project is to design and implement the essential components of an operating system, including:

- Memory management (kernel & user space)
- Page fault handling and replacement strategies
- Process scheduling
- Inter-process communication
- Kernel protection and synchronization

All components are validated using predefined and unseen test cases that simulate realistic execution scenarios.

---

## 🎯 Project Objectives

- Apply operating system theory in real kernel implementations
- Understand virtual memory and paging mechanisms
- Implement and compare multiple page replacement algorithms
- Design a priority-based CPU scheduler with starvation prevention
- Build safe kernel synchronization primitives
- Gain experience in debugging and testing low-level systems code

---

## 🧩 System Architecture

The system is divided into **Group Modules** (prerequisites) and **Individual Modules** to allow structured development and testing.

---

## 🔹 Group Modules (Prerequisites)

### 1. Dynamic Allocator
- Block-based dynamic memory allocation
- Allocation, deallocation, and block management

### 2. Kernel Heap
- Page allocator and block allocator
- Virtual to physical address translation
- Custom fit allocation strategy

### 3. Page Fault Handler I (Placement)
- Demand paging
- Stack growth handling
- Invalid memory access detection

---

## 🔹 Individual Modules

### 🧠 1. Page Fault Handler II & III (Replacement)
Implements multiple page replacement algorithms with full working set management:

- **Optimal (OPT)** – theoretical benchmark
- **Clock (Second Chance)**
- **LRU (Aging-based approximation)**
- **Modified Clock**

Responsibilities include:
- Tracking page reference streams
- Managing working sets
- Handling page eviction and disk interaction
- Maintaining used and modified bits

---

### 💾 2. User Heap Management
Provides dynamic memory allocation for user processes:

- Block allocator for small allocations
- Page allocator for large allocations
- **Custom Fit** allocation strategy
- Lazy allocation through page faults

User-level APIs:
- `malloc()`
- `free()`

Kernel support:
- `allocate_user_mem()`
- `free_user_mem()`

---

### 🔗 3. Shared Memory
Enables inter-process communication through shared memory objects:

- Runtime creation and sharing of memory objects
- Reference counting and permission control
- Frame tracking for safe sharing
- Full kernel-level synchronization using locks

User-level APIs:
- `smalloc()`
- `sget()`

---

### ⏱️ 4. CPU Scheduling
Implements a **Priority-Based Round Robin Scheduler**:

- Multiple ready queues per priority level
- Preemptive scheduling
- Configurable quantum
- Starvation prevention via priority promotion
- Runtime priority modification using system calls

---

### 🔐 5. Kernel Protection & Synchronization
Ensures safe concurrent execution inside the kernel using:

- Sleep Locks
- Semaphores
- Channel-based sleep/wakeup mechanisms

Used for:
- Console I/O protection
- Disk I/O synchronization
- Inter-process coordination

---

## 🧪 Testing & Validation

The project follows a **logic-driven design approach** rather than test-driven development.

Testing includes:
- Individual module testing
- Group module testing
- Full system integration testing
- Execution of real user programs (sorting, Fibonacci, shared memory workloads)

Each test has strict time limits and must complete **without kernel panics or errors** to be considered successful.

---

## ▶️ How to Run & Test

1. Enable or disable kernel heap when required:
   - Modify `USE_KHEAP` in `inc/memlayout.h`

2. Switch scheduling or replacement policies from the FOS prompt:
   - `optimal`, `clock`, `lru`, `modclock`
   - `schedPRIRR`, `schedRR`

3. Run a single program:
   ```bash
   FOS> run <program_name> <working_set_size> [priority]

4. Load and execute multiple programs:
   ```bash
   FOS> load <program_name> <working_set_size> [priority]
   FOS> runall

## 🛠 Technologies & Environment

- **Programming Language:** C  
- **Domain:** Operating Systems / Kernel Development  
- **Environment:** Educational OS Framework (FOS)  
- **Execution Model:** Monolithic Kernel  
- **Memory Model:** Paging & Virtual Memory  
- **Scheduling Model:** Priority-Based Round Robin  

---

## 📚 Learning Outcomes

Through this project, the developer gained practical experience in:

- Kernel-level memory management
- Page placement and replacement strategies
- Working set management
- CPU scheduling and starvation handling
- Synchronization and concurrency control
- Inter-process communication using shared memory
- Debugging and testing low-level operating system code

---

## ⚠️ Disclaimer

This project was developed strictly for **educational purposes** as part of an academic Operating Systems course at **FCIS**.  
It is not intended to be a production-ready operating system.

---

## 🤝 Team Acknowledgment

Special thanks to my amazing teammates for their collaboration, dedication, and continuous support throughout the development of this project.

**Team Members:**
- Omnia Mostafa
- Tag Eldeen
- Meshkat Zaki
- Sohila Mohamed
- Sama Waleed

---

## 🙏 Acknowledgment

Special thanks to Dr. **Ahmed Salah** for his guidance and support throughout the semester.
