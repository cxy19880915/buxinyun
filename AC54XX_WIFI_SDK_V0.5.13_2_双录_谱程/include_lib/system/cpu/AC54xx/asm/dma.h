#ifndef __CPU_DMA_H__
#define __CPU_DMA_H__



struct dma_list {
    void *src_addr;
    void *dst_addr;
    int len;
};



void dma_copy(void *dst, void *src, int len);

void dma_task_copy(struct dma_list *list, int cnt);





#endif
