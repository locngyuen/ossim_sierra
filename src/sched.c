
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++) {
		mlq_ready_queue[i].size = 0;
		
		// Khởi tạo slot cho mỗi mức ưu tiên
		// Trong MLQ thuần túy, không cần quan tâm đến số slot
		slot[i] = 0;
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);

	// Khởi tạo running_list
	running_list.size = 0;
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	
    pthread_mutex_lock(&queue_lock);

    /* Mô hình MLQ phù hợp với output chuẩn:
     * - Tiến trình trong hàng đợi có độ ưu tiên cao nhất được chạy trước
     * - Mọi CPU sẽ lấy tiến trình theo thứ tự ưu tiên
     * - Tiến trình có cùng độ ưu tiên được chạy theo FIFO
     * - Nếu mọi CPU đều đang chạy tiến trình, các tiến trình mới được chạy theo MLQ khi CPU rảnh
     */
    
    // Duyệt từ hàng đợi có độ ưu tiên cao đến thấp
    for (int prio = MAX_PRIO - 1; prio >= 0; prio--) {
        // Nếu hàng đợi không rỗng, lấy tiến trình từ đầu hàng đợi
        if (!empty(&mlq_ready_queue[prio])) {
            proc = dequeue(&mlq_ready_queue[prio]);
            pthread_mutex_unlock(&queue_lock);
            return proc;
        }
    }

    // Không tìm thấy tiến trình nào để chạy
    pthread_mutex_unlock(&queue_lock);
    return NULL;
}

/* Hàm này đưa tiến trình đang chạy trở lại ready queue của MLQ */

void put_mlq_proc(struct pcb_t *proc) {
    pthread_mutex_lock(&queue_lock);
    
    // Đặt tiến trình vào cuối hàng đợi có cùng mức độ ưu tiên
    // Đảm bảo các tiến trình có cùng độ ưu tiên được xử lý theo FIFO
    enqueue(&mlq_ready_queue[proc->prio], proc);
    
    pthread_mutex_unlock(&queue_lock);
}



/* Hàm thêm tiến trình mới vào ready queue của MLQ */

void add_mlq_proc(struct pcb_t *proc) {
    // Kiểm tra tính hợp lệ của độ ưu tiên
    if (proc == NULL || proc->prio < 0 || proc->prio >= MAX_PRIO) {
        fprintf(stderr, "Error: Invalid priority when adding process\n");
        return;
    }

    pthread_mutex_lock(&queue_lock);

    // Thêm tiến trình vào hàng đợi tương ứng với mức ưu tiên của nó
    enqueue(&mlq_ready_queue[proc->prio], proc);
    
    pthread_mutex_unlock(&queue_lock);    
}



struct pcb_t * get_proc(void) {

    return get_mlq_proc();

}



/* Khi tiến trình đang chạy cần được đưa ra khỏi CPU (preemption) 

 * ta đưa nó vào running_list để theo dõi và đồng thời đưa về hàng đợi MLQ.

 */

void put_proc(struct pcb_t *proc) {
    // Kiểm tra NULL pointer
    if (proc == NULL) {
        return;
    }

    proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;
    
    // Thêm tiến trình đang chạy vào running_list để theo dõi
    pthread_mutex_lock(&queue_lock);
    
    // Kiểm tra xem tiến trình đã có trong running_list chưa để tránh trùng lặp
    int found = 0;
    for (int i = 0; i < running_list.size; i++) {
        if (running_list.proc[i] == proc) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        enqueue(&running_list, proc);
    }
    
    pthread_mutex_unlock(&queue_lock);

    // Đưa tiến trình về hàng đợi sẵn sàng theo MLQ
    put_mlq_proc(proc);
}



/* Khi tiến trình mới được tạo, cần được đưa vào running_list và ready queue MLQ */

void add_proc(struct pcb_t *proc) {
    // Kiểm tra NULL pointer
    if (proc == NULL) {
        return;
    }

    proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;

    // In thông tin tiến trình đã được thêm vào
    printf("Added new process PID: %d with priority: %d\n", proc->pid, proc->prio);

    // Thêm tiến trình mới vào running_list để theo dõi
    pthread_mutex_lock(&queue_lock);
    
    // Kiểm tra xem tiến trình đã có trong running_list chưa để tránh trùng lặp
    int found = 0;
    for (int i = 0; i < running_list.size; i++) {
        if (running_list.proc[i] == proc) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        enqueue(&running_list, proc);
    }
    
    pthread_mutex_unlock(&queue_lock);

    // Đưa tiến trình vào hàng đợi MLQ
    add_mlq_proc(proc);
}

#else

/* Nếu MLQ_SCHED không được định nghĩa, cài đặt các hàm theo cách thông thường */

struct pcb_t * get_proc(void) {

    struct pcb_t * proc = NULL;

    /*TODO: get a process from [ready_queue].

     * Remember to use lock to protect the queue.

     * */

    pthread_mutex_lock(&queue_lock);
    
    if (!empty(&ready_queue)) {
        proc = dequeue(&ready_queue);
    } else if (!empty(&run_queue)) {
        proc = dequeue(&run_queue);
    }
     
     pthread_mutex_unlock(&queue_lock);
     
     return proc;

}



void put_proc(struct pcb_t * proc) {

    proc->ready_queue = &ready_queue;

    proc->running_list = &running_list;

    /* TODO: put running proc to running_list */

    pthread_mutex_lock(&queue_lock);

    enqueue(&run_queue, proc);

    pthread_mutex_unlock(&queue_lock);

}



void add_proc(struct pcb_t * proc) {

    proc->ready_queue = &ready_queue;

    proc->running_list = &running_list;

    /* TODO: put running proc to running_list */

    pthread_mutex_lock(&queue_lock);

    enqueue(&ready_queue, proc);

    pthread_mutex_unlock(&queue_lock);    

}

#endif


