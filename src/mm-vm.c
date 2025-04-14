// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

 #include "string.h"
 #include "mm.h"
 #include <stdlib.h>
 #include <stdio.h>
 #include <pthread.h>
 
 /*get_vma_by_num - get vm area by numID
  *@mm: memory region
  *@vmaid: ID vm area to alloc memory region
  *
  */
 struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
 {
 #ifdef VMDBG
   printf("VMDBG: get_vma_by_num(mm=%p, vmaid=%d) - Enter\n", mm, vmaid);
 #endif
   if (mm == NULL || mm->mmap == NULL) {
 #ifdef VMDBG
     printf("VMDBG: get_vma_by_num - NULL mm or mm->mmap. Exit NULL.\n");
 #endif
     return NULL;
   }

   struct vm_area_struct *pvma = mm->mmap;
   
   // Nếu vmaid là 0, trả về vma đầu tiên ngay lập tức
   if (vmaid == 0)
     return pvma;
     
   // Vòng lặp để tìm VMA theo ID
   while (pvma != NULL) {
     if (pvma->vm_id == vmaid) {
 #ifdef VMDBG
       printf("VMDBG: get_vma_by_num - Found pvma=%p for vmaid=%d. Exit.\n", pvma, vmaid);
 #endif
       return pvma;
     }
     
     pvma = pvma->vm_next;
   }

 #ifdef VMDBG
   printf("VMDBG: get_vma_by_num - VMA with id=%d not found. Exit NULL.\n", vmaid);
 #endif
   return NULL; // Không tìm thấy VMA với ID tương ứng
 }
 
 int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
 {
     __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
     return 0;
 }
 
 /*get_vm_area_node - get vm area for a number of pages
  *@caller: caller
  *@vmaid: ID vm area to alloc memory region
  *@incpgnum: number of page
  *@vmastart: vma end
  *@vmaend: vma end
  *
  */
 struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
 {
   struct vm_rg_struct * newrg;
   /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
   // struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
   //Lấy thông tin về vùng nhớ ảo hiện tại
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
 
   if (cur_vma == NULL)
     return NULL;
 
   newrg = malloc(sizeof(struct vm_rg_struct));
 
   /* TODO: update the newrg boundary
   // newrg->rg_start = ...
   // newrg->rg_end = ...
   */
   if(newrg == NULL)
     return NULL;
   
   // cập nhật ranh giới của vùng nhớ mới
   newrg->rg_start = cur_vma->vm_end;
   newrg->rg_end = cur_vma->vm_end + alignedsz;
 
   // cập nhật vị trí break mới cho vùng nhớ ảo 
   cur_vma->sbrk = newrg->rg_end;
 
   // đảm bảo vị trí break không vượt quá giới hạn vm_end
   if(cur_vma->sbrk > cur_vma->vm_end){
     cur_vma->vm_end = cur_vma->sbrk;
   }
 
   return newrg;
 }
 
 /*validate_overlap_vm_area
  *@caller: caller
  *@vmaid: ID vm area to alloc memory region
  *@vmastart: vma end
  *@vmaend: vma end
  *
  */
 int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
 {
 #ifdef VMDBG
   printf("VMDBG: validate_overlap_vm_area(PID=%d, vmaid=%d, start=0x%x, end=0x%x) - Enter\n", caller->pid, vmaid, vmastart, vmaend);
 #endif
   //struct vm_area_struct *vma = caller->mm->mmap;
   if(vmastart >= vmaend || caller == NULL) {
 #ifdef VMDBG
     printf("VMDBG: validate_overlap_vm_area - Invalid args (start>=end or caller null). Exit -1.\n");
 #endif
     return -1;
   }
 
   struct vm_area_struct *vma = caller->mm->mmap;
   /* TODO validate the planned memory area is not overlapped */
   while(vma != NULL){
 #ifdef VMDBG
     printf("VMDBG: validate_overlap_vm_area - Checking against vma_id=%lu [0x%lx - 0x%lx]\n", vma->vm_id, vma->vm_start, vma->vm_end);
 #endif
     if(vma->vm_id == vmaid) {
        vma = vma->vm_next; // Skip self
        continue;
     }
     if ((vmastart >= vma->vm_start && vmastart < vma->vm_end) ||  //start đè lên bộ nhớ hiện có 
         (vmaend > vma->vm_start && vmaend <= vma->vm_end)     ||  // end đè lên bộ nhớ hiện có 
         (vmastart <= vma->vm_start && vmaend >= vma->vm_end)  ||  //new mem chứa old mem
         (vmastart >= vma->vm_start && vmaend <= vma->vm_end))     //old mem chúa new mem
         {
 #ifdef VMDBG
           printf("VMDBG: validate_overlap_vm_area - Overlap detected with vma_id=%lu. Exit -1.\n", vma->vm_id);
 #endif
           return -1;
         }
     vma = vma->vm_next;
   }
 #ifdef VMDBG
   printf("VMDBG: validate_overlap_vm_area - No overlap detected. Exit 0.\n");
 #endif
   return 0;  //khong co xung dot
 }
 
 /*inc_vma_limit - increase vm area limits to reserve space for new variable
  *@caller: caller
  *@vmaid: ID vm area to alloc memory region
  *@inc_sz: increment size
  *
  */
 int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
 {
 #ifdef VMDBG
   printf("VMDBG: inc_vma_limit(PID=%d, vmaid=%d, inc_sz=%d) - Enter\n", caller->pid, vmaid, inc_sz);
 #endif
   struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
   int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);     //làm tròn kích thước tăng lên theo đơn vị trang 
   int incnumpage =  inc_amt / PAGING_PAGESZ;     // Tính số lượng trang cần thêm
   struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
 
   int old_end = cur_vma->vm_end;
 
   //kiểm tra tham số đầu vào trước khi thực thi (tự thêm)
   if(caller == NULL || cur_vma == NULL || area == NULL || newrg == NULL ){
 #ifdef VMDBG
     printf("VMDBG: inc_vma_limit - Invalid args (caller=%p, cur_vma=%p, area=%p, newrg=%p). Exit -1.\n", caller, cur_vma, area, newrg);
 #endif
     if(newrg != NULL) free(newrg);
     if(area != NULL) free(area);
     return -1;
   }
 
   /*Validate overlap of obtained region */
   int ret_val_overlap = validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end);
   if (ret_val_overlap < 0)
   {
 #ifdef VMDBG
     printf("VMDBG: inc_vma_limit - Overlap validation failed (%d). Freeing area=%p, newrg=%p. Exit -1.\n", ret_val_overlap, area, newrg);
 #endif
     free(area); 
     free(newrg);
     return -1; /*Overlap and failed allocation */
   }
 
   /* TODO: Obtain the new vm area based on vmaid */
   //cur_vma->vm_end... 
   // inc_limit_ret...
   cur_vma->vm_end = area->rg_end; //mở rộng giới hạn cuối vùng nhớ
   
   // Cập nhật vị trí break nếu cần thiết - thử ko cần có làm sao ko 
   if (cur_vma->sbrk < cur_vma->vm_end) {
     cur_vma->sbrk = cur_vma->vm_end;
   }
 
 #ifdef VMDBG
     printf("VMDBG: inc_vma_limit - Calling vm_map_ram(PID=%d, start=0x%lx, end=0x%lx, old_end=0x%x, numpg=%d, newrg=%p)\n", 
            caller->pid, area->rg_start, area->rg_end, old_end, incnumpage, newrg);
 #endif
   if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                     old_end, incnumpage , newrg) < 0){
 #ifdef VMDBG
     printf("VMDBG: inc_vma_limit - vm_map_ram failed. Restoring vm_end to 0x%x. Freeing area=%p, newrg=%p. Exit -1.\n", old_end, area, newrg);
 #endif
     // Khôi phục lại giới hạn cũ nếu ánh xạ thất bại
     cur_vma->vm_end = old_end;
     free(newrg);
     free(area);
     return -1; /* Map the memory to MEMRAM */
   }
 
   //free(area); - check lai xem co can hog
 #ifdef VMDBG
     printf("VMDBG: inc_vma_limit - Successfully increased limit and mapped RAM. Freeing area=%p. Exit 0.\n", area); // Potential free(area) needed?
 #endif
     free(area); // Added free(area) here as it seems necessary
   return 0;
 }
 
 // #endif
 