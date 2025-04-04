/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "string.h"
#include "queue.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queu
    struct queue_t *running_list = caller->running_list;
    int count = 0;

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */
    if (running_list != NULL) {
        for (i = 0; i < running_list->size; i++) {
            struct pcb_t *proc = running_list->proc[i];
            if (proc != NULL && strcmp(proc->path, proc_name) == 0 && proc->pid != caller->pid) {
                // Đánh dấu tiến trình để kết thúc
                proc->pc = proc->code->size; // Đặt program counter vượt quá kích thước code
                count++;
            }
        }
    }

#ifdef MLQ_SCHED
    // Duyệt qua tất cả các hàng đợi với độ ưu tiên khác nhau
    for (int prio = 0; prio < MAX_PRIO; prio++) {
        struct queue_t *mlq_queue = &caller->mlq_ready_queue[prio];
        for (i = 0; i < mlq_queue->size; i++) {
            struct pcb_t *proc = mlq_queue->proc[i];
            if (proc != NULL && strcmp(proc->path, proc_name) == 0 && proc->pid != caller->pid) {
                proc->pc = proc->code->size;
                count++;
            }
        }
    }
#else
    // Kiểm tra trong ready_queue
    struct queue_t *ready_queue = caller->ready_queue;
    if (ready_queue != NULL) {
        for (i = 0; i < ready_queue->size; i++) {
            struct pcb_t *proc = ready_queue->proc[i];
            if (proc != NULL && strcmp(proc->path, proc_name) == 0 && proc->pid != caller->pid) {
                proc->pc = proc->code->size;
                count++;
            }
        }
    }
#endif

    printf("Đã kết thúc %d tiến trình có tên \"%s\"\n", count, proc_name);
    return count; 
}
