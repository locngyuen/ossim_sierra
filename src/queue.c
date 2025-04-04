#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q == NULL || proc == NULL) return;
        if (q->size == MAX_QUEUE_SIZE) return; // Nếu hàng đợi đã đầy

        q->proc[q->size] = proc; // Thêm tiến trình vào cuối hàng đợi
        q->size++; // Tăng kích thước hàng đợi
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (q == NULL || q->size == 0) return NULL; // Nếu hàng đợi rỗng
        
        // Tìm tiến trình có độ ưu tiên cao nhất
        int highest_prio_idx = 0;
        for (int i = 1; i < q->size; i++) {
            if (q->proc[i]->priority > q->proc[highest_prio_idx]->priority) {
                highest_prio_idx = i;
            }
        }
        
        // Lấy tiến trình có độ ưu tiên cao nhất
        struct pcb_t * proc = q->proc[highest_prio_idx];
        
        // Xóa tiến trình đã lấy khỏi hàng đợi
        for (int i = highest_prio_idx; i < q->size - 1; i++) {
            q->proc[i] = q->proc[i + 1];
        }
        
        q->size--; // Giảm kích thước hàng đợi
        
        return proc;
}

