#include "mem_blocks.h"


GST_DEBUG_CATEGORY_STATIC(test_vpu_mem_blocks_debug);
#define GST_CAT_DEFAULT test_vpu_mem_blocks_debug


static void setup_debug_category(void)
{
	static gsize initialized = 0;

	if (g_once_init_enter(&initialized))
	{
		GST_DEBUG_CATEGORY_INIT(test_vpu_mem_blocks_debug, "testvpumemblocks", 0, "VPU memory block functions");
		g_once_init_leave(&initialized, 1);
	}
}


gboolean gst_test_vpu_alloc_virt_mem_block(unsigned char **mem_block, int size)
{
	setup_debug_category();

	*mem_block = (unsigned char *)g_try_malloc(size);
	if ((*mem_block) == NULL)
	{
		GST_ERROR("could not request %d bytes of heap memory", size);
		return FALSE;
	}
	else
		GST_INFO("allocated %d bytes of heap memory at virt addr %p", size, *mem_block);

	return TRUE;
}

// 挂载到链表中
void gst_test_vpu_append_virt_mem_block(unsigned char *mem_block, GSList **virt_mem_blocks)
{
	setup_debug_category();

	*virt_mem_blocks = g_slist_append(*virt_mem_blocks, (gpointer)mem_block);
}


gboolean gst_test_vpu_free_virt_mem_blocks(GSList **virt_mem_blocks)
{
	GSList *mem_block_node;

	setup_debug_category();

	g_assert(virt_mem_blocks != NULL);

	mem_block_node = *virt_mem_blocks;
	if (mem_block_node == NULL)
		return TRUE;

	for (; mem_block_node != NULL; mem_block_node = mem_block_node->next)
	{
		g_free(mem_block_node->data);
		GST_INFO("freed heap memory block at virt addr %p", mem_block_node->data);
	}

	g_slist_free(*virt_mem_blocks);
	*virt_mem_blocks = NULL;

	return TRUE;
}

//将memory挂载到phys_mem_blocks链表中
void gst_test_vpu_append_phys_mem_block(GstTestPhysMemory *memory, GSList **phys_mem_blocks)
{
	setup_debug_category();

	*phys_mem_blocks = g_slist_append(*phys_mem_blocks, (gpointer)memory);
}


gboolean gst_test_vpu_free_phys_mem_blocks(GstTestPhysMemAllocator *phys_mem_allocator, GSList **phys_mem_blocks)
{
	GSList *mem_block_node;

	setup_debug_category();

	g_assert(phys_mem_blocks != NULL);

	mem_block_node = *phys_mem_blocks;
	if (mem_block_node == NULL)
		return TRUE;

	for (; mem_block_node != NULL; mem_block_node = mem_block_node->next)
	{
		GstMemory *memory = (GstMemory *)(mem_block_node->data);
		GST_INFO("freed phys memory block with %u bytes at phys addr %" GST_TEST_PHYS_ADDR_FORMAT, memory->size, ((GstTestPhysMemory *)memory)->phys_addr);
		gst_memory_unref(memory);
	}

	g_slist_free(*phys_mem_blocks);
	*phys_mem_blocks = NULL;

	return TRUE;
}
