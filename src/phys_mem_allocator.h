#ifndef GST_TEST_COMMON_PHYS_MEM_ALLOCATOR_H
#define GST_TEST_COMMON_PHYS_MEM_ALLOCATOR_H

#include <gst/gst.h>
#include <gst/gstallocator.h>

#ifndef GST_TEST_COMMON_PHYS_MEM_ADDR_H
#define GST_TEST_COMMON_PHYS_MEM_ADDR_H

#define GST_TEST_PHYS_ADDR_FORMAT "#lx"
typedef unsigned long gst_test_phys_addr_t;

G_BEGIN_DECLS



typedef struct _GstTestPhysMemAllocator GstTestPhysMemAllocator;
typedef struct _GstTestPhysMemAllocatorClass GstTestPhysMemAllocatorClass;
typedef struct _GstTestPhysMemory GstTestPhysMemory;


#define GST_TYPE_TEST_PHYS_MEM_ALLOCATOR             (gst_test_phys_mem_allocator_get_type())
#define GST_TEST_PHYS_MEM_ALLOCATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TEST_PHYS_MEM_ALLOCATOR, GstTestPhysMemAllocator))
#define GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TEST_PHYS_MEM_ALLOCATOR, GstTestPhysMemAllocatorClass))
#define GST_IS_TEST_PHYS_MEM_ALLOCATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TEST_PHYS_MEM_ALLOCATOR))
#define GST_IS_TEST_PHYS_MEM_ALLOCATOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TEST_PHYS_MEM_ALLOCATOR))


struct _GstTestPhysMemAllocator
{
	GstAllocator parent;
};


struct _GstTestPhysMemAllocatorClass
{
	GstAllocatorClass parent_class;

	gboolean (*alloc_phys_mem)(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory, gssize size);
	gboolean (*free_phys_mem)(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory);
	gpointer (*map_phys_mem)(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory, gssize size, GstMapFlags flags);
	void (*unmap_phys_mem)(GstTestPhysMemAllocator *allocator, GstTestPhysMemory *memory);
};


struct _GstTestPhysMemory
{
	GstMemory mem;

	gpointer mapped_virt_addr;
	gst_test_phys_addr_t phys_addr;

	GstMapFlags mapping_flags;

	/* Counter to ensure the memory block isn't (un)mapped
	 * more often than necessary */
	long mapping_refcount;

	/* pointer for any additional internal data an allocator may define
	 * not for outside use; allocators do not have to use it */
	gpointer internal;
};


GType gst_test_phys_mem_allocator_get_type(void);

guintptr gst_test_phys_memory_get_phys_addr(GstMemory *mem);
gboolean gst_test_is_phys_memory(GstMemory *mem);


G_END_DECLS


#endif
