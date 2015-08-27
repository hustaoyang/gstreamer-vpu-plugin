#include <string.h>
#include "vpu_wrapper.h"
#include "allocator.h"
#include "decoder.h"


GST_DEBUG_CATEGORY_STATIC(test_vpu_dec_allocator_debug);
#define GST_CAT_DEFAULT test_vpu_dec_allocator_debug



static void gst_test_vpu_dec_allocator_finalize(GObject *object);

static gboolean gst_test_vpu_dec_alloc_phys_mem(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory, gssize size);
static gboolean gst_test_vpu_dec_free_phys_mem(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory);
static gpointer gst_test_vpu_dec_map_phys_mem(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory, gssize size, GstMapFlags flags);
static void gst_test_vpu_dec_unmap_phys_mem(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory);


G_DEFINE_TYPE(GstTestVpuDecAllocator, gst_test_vpu_dec_allocator, GST_TYPE_TEST_PHYS_MEM_ALLOCATOR)



//gstreamer 框架中类型分配
GstAllocator* gst_test_vpu_dec_allocator_new(void)
{
	GstAllocator *allocator;
	allocator = g_object_new(gst_test_vpu_dec_allocator_get_type(), NULL);

	return allocator;
}


static gboolean gst_test_vpu_dec_alloc_phys_mem(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory, gssize size)
{
	VpuDecRetCode ret;
	VpuMemDesc mem_desc;

	if (!gst_test_vpu_dec_load())
		return FALSE;

	memset(&mem_desc, 0, sizeof(VpuMemDesc));
	mem_desc.nSize = size;
	ret = VPU_DecGetMem(&mem_desc);

	if (ret == VPU_DEC_RET_SUCCESS)
	{
		memory->mem.size         = mem_desc.nSize;
		memory->mapped_virt_addr = (gpointer)(mem_desc.nVirtAddr);
		memory->phys_addr        = (gst_test_phys_addr_t)(mem_desc.nPhyAddr);
		memory->internal         = (gpointer)(mem_desc.nCpuAddr);

		GST_DEBUG_OBJECT(allocator, "addresses: virt: %p phys: %" GST_TEST_PHYS_ADDR_FORMAT " cpu: %p", memory->mapped_virt_addr, memory->phys_addr, memory->internal);

		return TRUE;
	}
	else
		return FALSE;
}


static gboolean gst_test_vpu_dec_free_phys_mem(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory)
{
    VpuDecRetCode ret;
    VpuMemDesc mem_desc;

	memset(&mem_desc, 0, sizeof(VpuMemDesc));
	mem_desc.nSize     = memory->mem.size;
	mem_desc.nVirtAddr = (unsigned long)(memory->mapped_virt_addr);
	mem_desc.nPhyAddr  = (unsigned long)(memory->phys_addr);
	mem_desc.nCpuAddr  = (unsigned long)(memory->internal);

	GST_DEBUG_OBJECT(allocator, "addresses: virt: %p phys: %" GST_TEST_PHYS_ADDR_FORMAT " cpu: %p", memory->mapped_virt_addr, memory->phys_addr, memory->internal);

	ret = VPU_DecFreeMem(&mem_desc);

	gst_test_vpu_dec_unload();

	return (ret == VPU_DEC_RET_SUCCESS);
}


static gpointer gst_test_vpu_dec_map_phys_mem(G_GNUC_UNUSED GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory, G_GNUC_UNUSED gssize size, G_GNUC_UNUSED GstMapFlags flags)
{
	return memory->mapped_virt_addr;
}


static void gst_test_vpu_dec_unmap_phys_mem(G_GNUC_UNUSED GstTestPhysMemAllocator *allocator, G_GNUC_UNUSED GstTestPhysMemory *memory)
{
}




static void gst_test_vpu_dec_allocator_class_init(GstTestVpuDecAllocatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GstTestPhysMemAllocatorClass *parent_class = GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(klass);

	object_class->finalize       = GST_DEBUG_FUNCPTR(gst_test_vpu_dec_allocator_finalize);
	parent_class->alloc_phys_mem = GST_DEBUG_FUNCPTR(gst_test_vpu_dec_alloc_phys_mem);
	parent_class->free_phys_mem  = GST_DEBUG_FUNCPTR(gst_test_vpu_dec_free_phys_mem);
	parent_class->map_phys_mem   = GST_DEBUG_FUNCPTR(gst_test_vpu_dec_map_phys_mem);
	parent_class->unmap_phys_mem = GST_DEBUG_FUNCPTR(gst_test_vpu_dec_unmap_phys_mem);

	GST_DEBUG_CATEGORY_INIT(test_vpu_dec_allocator_debug, "testvpudecallocator", 0, "VPU decoder physical memory/allocator");
}


static void gst_test_vpu_dec_allocator_init(GstTestVpuDecAllocator *allocator)
{
	GstAllocator *base = GST_ALLOCATOR(allocator);
	base->mem_type = GST_TEST_VPU_DEC_ALLOCATOR_MEM_TYPE;
}


static void gst_test_vpu_dec_allocator_finalize(GObject *object)
{
	GST_INFO_OBJECT(object, "shutting down TEST VPU decoder allocator");
	G_OBJECT_CLASS(gst_test_vpu_dec_allocator_parent_class)->finalize(object);
}
