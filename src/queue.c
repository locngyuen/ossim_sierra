#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/* Kiểm tra xem hàng đợi có rỗng không */
int empty(struct queue_t *q) {
    if (q == NULL)
        return 1;
    return (q->size == 0);
}

/* Thêm tiến trình (PCB) vào hàng đợi [q]
 * Nếu hàng đợi đã đầy (MAX_QUEUE_SIZE), in ra thông báo lỗi.
 */
void enqueue(struct queue_t *q, struct pcb_t *proc) {
/* TODO: put a new process to queue [q] */
    if (q == NULL || proc == NULL)
        return;

    if (q->size >= MAX_QUEUE_SIZE) {
        return;
    }
    
    q->proc[q->size] = proc;
    q->size++;
}

/* Lấy ra và loại bỏ tiến trình có độ ưu tiên cao nhất trong hàng đợi [q]
 * Trong MLQ_SCHED: giá trị prio càng nhỏ thì ưu tiên càng cao.
 * Trong scheduling thường: giá trị priority càng lớn thì ưu tiên càng cao.
 */
struct pcb_t *dequeue(struct queue_t *q) {
/* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
    if (empty(q))
        return NULL;
    
    int highestIndex = 0;
    int i;
#ifdef MLQ_SCHED
    // Trong MLQ_SCHED, số prio càng nhỏ thì ưu tiên càng cao
    for (i = 1; i < q->size; i++) {
        if (q->proc[i]->prio < q->proc[highestIndex]->prio)
            highestIndex = i;
    }
#else
    // Trong scheduling thường, số priority càng lớn thì ưu tiên càng cao
    for (i = 1; i < q->size; i++) {
        if (q->proc[i]->priority > q->proc[highestIndex]->priority)
            highestIndex = i;
    }
#endif

    struct pcb_t *selected = q->proc[highestIndex];
    
    /* Dịch chuyển các phần tử sau vị trí selected về phía trước */
    for (i = highestIndex; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i + 1];
    }
    q->size--;
    
    return selected;
}

