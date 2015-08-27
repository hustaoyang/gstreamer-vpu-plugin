#ifndef GST_TEST_PHYS_MEM_BLOCKS_H
#define GST_TEST_PHYS_MEM_BLOCKS_H

#include <gst/gst.h>
#include "phys_mem_allocator.h"


G_BEGIN_DECLS


gboolean gst_test_vpu_alloc_virt_mem_block(unsigned char **mem_block, int size);
void gst_test_vpu_append_virt_mem_block(unsigned char *mem_block, GSList **virt_mem_blocks);
gboolean gst_test_vpu_free_virt_mem_blocks(GSList **virt_mem_blocks);



// 没有gst_test_vpu_alloc_phys_mem_block（）函数
// 物理内存在VPU 中不需要分配
void gst_test_vpu_append_phys_mem_block(GstTestPhysMemory *memory, GSList **phys_mem_blocks);
gboolean gst_test_vpu_free_phys_mem_blocks(GstTestPhysMemAllocator *phys_mem_allocator, GSList **phys_mem_blocks);


G_END_DECLS


#endif

