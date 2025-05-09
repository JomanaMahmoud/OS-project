# 🧠 Operating System Simulation (CSEN 602 – Milestone 1 & 2)

## 📌 Overview

This project is part of the **Operating Systems course (CSEN 602)** at the **German University in Cairo (GUC)** during **Spring 2025**, developed by **Team 64**.

It consists of two main milestones that simulate various core OS concepts:
- **Milestone 1**: Thread creation, scheduling algorithms, and performance analysis.
- **Milestone 2**: Full OS simulation with custom scheduling, memory management, mutex-based synchronization, and a GTK GUI.

---

## ⚙️ Milestone 1 — Threads & Scheduling Analysis

### 🧵 Thread Functionality

- **Thread 1**: Takes two alphabetic characters and prints all letters in-between (inclusive).
- **Thread 2**: Prints messages and its thread ID.
- **Thread 3**: Accepts two integers and prints their sum, average, and product.

### 📋 Scheduling Algorithms

- Experimented with **predefined scheduling policies** (e.g., Round Robin, FIFO).
- Used **pthreads** for concurrency.
- Measured and analyzed:
  - Execution time
  - Waiting time
  - Turnaround time
  - CPU utilization
  - Memory usage

> This milestone laid the foundation for process management and scheduling in later milestones.

---

## 🛠️ Milestone 2 — OS Simulation

### 🧵 Process Scheduling

- FCFS, Round Robin (user-defined quantum), and MLFQ (4 levels, quantum doubles)

### 🧠 Memory Management

- 60 memory cells with bounds per process
- Dynamic process loading at arrival

### 🔒 Mutual Exclusion with Mutexes

- Mutexes for `userInput`, `userOutput`, and `file`
- `semWait` / `semSignal` behavior blocks and unblocks processes by priority

### 📄 Instruction Interpreter

- Supports: `assign`, `print`, `readFile`, `writeFile`, `semWait`, `semSignal`, `printFromTo`

### 🖼️ GTK GUI

- Scheduler config tab, memory viewer, mutex state viewer, real-time dashboard, output logs

---

## 🧱 Tech Stack

| Layer         | Technology     |
|---------------|----------------|
| Backend       | C              |
| Frontend      | GTK (GTK3)     |
| Threads       | Pthreads (M1)  |
| Compiler      | GCC            |
| OS Support    | Ubuntu / WSL   |
| IDE (optional)| VS Code        |

---

## 💻 Installation & Setup

### Ubuntu Native

1. **Install Dependencies**

```bash
sudo apt update
sudo apt install build-essential libgtk-3-dev
````

2. **Compile the Project**

Make sure you're in the project root directory, then run:

```bash
gcc main.c dashboard.c process_scheduler_tab.c memory_viewer_tab.c mutex_tab.c -o app $(pkg-config --cflags --libs gtk+-3.0)
```

> Add any additional `.c` files your project uses as needed.

3. **Run the Program**

```bash
./app
```

---

### WSL (Windows Subsystem for Linux) + VS Code

1. **Install WSL**:

```bash
wsl --install -d Ubuntu
```

2. **Install dependencies inside Ubuntu terminal**:

```bash
sudo apt update
sudo apt install build-essential libgtk-3-dev
```

3. **Open project in VS Code with Remote-WSL**:

```bash
code .
```

4. **Compile and Run as above**

---

## 🧪 Sample Instructions

```txt
assign x 5
assign y input
semWait userInput
printFromTo x y
semSignal userInput
semWait file
writeFile result.txt x
semSignal file
```

---

## 👨‍💻 Team 64 – Spring 2025

* **Aesha Anwar Sherif** — 58-0464
* **Walid Moussa Khalil** — 58-1001
* **Jomana Mahmoud Abdelmigid** — 58-1034
* **Sara Ahmed Elsawy** — 58-1857
* **Yehia Hassan Sadek** — 58-2571
* **Nada Yasser** — 58-3703
* **Rawan Hossam** — 58-25160

**Supervisors**:

* Dr. Eng. Catherine M. Elias
* Dr. Eng. Aya Salama

---

## 📜 License

Academic project for CSEN 602 — German University in Cairo




