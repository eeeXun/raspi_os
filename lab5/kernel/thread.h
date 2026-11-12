#pragma once

#include "list.h"
#include "syscall.h"

typedef struct {
    unsigned long long x19_x28[10];
    unsigned long long fp; // x29
    unsigned long long lr; // x30, where ret returns to
    unsigned long long sp;
} cpu_context;

typedef enum {
    RUNNING,
    DEAD,
} thread_status;

typedef struct {
    void* data;
    int ref_count;
} user_program_t;

typedef struct thread_t {
    cpu_context context;
    thread_status status;
    int tid;
    void (*func)(void*);
    void* arg;
    void* kernel_stack;
    user_program_t* user_program;
    void* user_stack;
    list_head list_node;
} thread_t;

extern void switch_to(thread_t* current, thread_t* next);
extern thread_t* get_current_thread();

user_program_t* user_program_load(void* data, int size);
void user_program_put(user_program_t* user_program);
void schedule();
void thread_exit();
void thread_kill(int tid);
int thread_alive(int tid);
void thread_wait(int tid);
int thread_fork(trap_frame* tf);
thread_t* thread_create(void (*func)(void*), void* arg);
void thread_init();
