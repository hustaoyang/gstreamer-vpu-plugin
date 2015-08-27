/*
 GstAllocator — allocate memory blocks
 struct GstAllocatorClass {
  GstObjectClass object_class;

  GstMemory *  (*alloc)      (GstAllocator *allocator, gsize size,
                              GstAllocationParams *params);
  void         (*free)       (GstAllocator *allocator, GstMemory *memory);
};

	GstAllocator
	typedef struct {
	  GstMemoryMapFunction       mem_map;
	  GstMemoryUnmapFunction     mem_unmap;

	  GstMemoryCopyFunction      mem_copy;
	  GstMemoryShareFunction     mem_share;
	  GstMemoryIsSpanFunction    mem_is_span;

	  GstMemoryMapFullFunction   mem_map_full;
	  GstMemoryUnmapFullFunction mem_unmap_full;
	} GstAllocator;

 GstMemory — refcounted wrapper for memory blocks
struct GstMemory {
  GstMiniObject   mini_object;

  GstAllocator   *allocator;

  GstMemory      *parent;
  gsize           maxsize;
  gsize           align;
  gsize           offset;
  gsize           size;
};
 
 enum GstMapFlags
	Flags used when mapping memory
	Members
	GST_MAP_READ
	map for read access
	GST_MAP_WRITE
	map for write access
	GST_MAP_FLAG_LAST
	first flag that can be used for custom purposes

 */

#include <string.h>
#include "phys_mem_allocator.h"


GST_DEBUG_CATEGORY_STATIC(test_phys_mem_allocator_debug);
#define GST_CAT_DEFAULT test_phys_mem_allocator_debug


static void gst_test_phys_mem_allocator_finalize(GObject *object);
static GstMemory* gst_test_phys_mem_allocator_alloc(GstAllocator *allocator, gsize size, GstAllocationParams *params);
static void gst_test_phys_mem_allocator_free(GstAllocator *allocator, GstMemory *memory);
static gpointer gst_test_phys_mem_allocator_map(GstMemory *mem, gsize maxsize, GstMapFlags flags);
static void gst_test_phys_mem_allocator_unmap(GstMemory *mem);
static GstMemory* gst_test_phys_mem_allocator_copy(GstMemory *mem, gssize offset, gssize size);
static GstMemory* gst_test_phys_mem_allocator_share(GstMemory *mem, gssize offset, gssize size);
static gboolean gst_test_phys_mem_allocator_is_span(GstMemory *mem1, GstMemory *mem2, gsize *offset);


G_DEFINE_ABSTRACT_TYPE(GstTestPhysMemAllocator, gst_test_phys_mem_allocator, GST_TYPE_ALLOCATOR)


void gst_test_phys_mem_allocator_class_init(GstTestPhysMemAllocatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GstAllocatorClass *parent_class = GST_ALLOCATOR_CLASS(klass);

	GST_DEBUG_CATEGORY_INIT(test_phys_mem_allocator_debug, "testphysmemallocator", 0, "Allocator for physically contiguous memory blocks");

	klass->alloc_phys_mem  = NULL;
	klass->free_phys_mem   = NULL;
	klass->map_phys_mem    = NULL;
	klass->unmap_phys_mem  = NULL;
	parent_class->alloc    = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_alloc);
	parent_class->free     = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_free);
	object_class->finalize = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_finalize);
}


void gst_test_phys_mem_allocator_init(GstTestPhysMemAllocator *allocator)
{
	GstAllocator *parent = GST_ALLOCATOR(allocator);

	GST_INFO_OBJECT(allocator, "initializing physical memory allocator");

	parent->mem_type    = NULL;
	parent->mem_map     = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_map);
	parent->mem_unmap   = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_unmap);
	parent->mem_copy    = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_copy);
	parent->mem_share   = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_share);
	parent->mem_is_span = GST_DEBUG_FUNCPTR(gst_test_phys_mem_allocator_is_span);
}

//未使用
static void gst_test_phys_mem_allocator_finalize(GObject *object)
{
	GST_INFO_OBJECT(object, "shutting down physical memory allocator");
	G_OBJECT_CLASS (gst_test_phys_mem_allocator_parent_class)->finalize(object);
}


static GstTestPhysMemory* gst_test_phys_mem_new_internal(GstTestPhysMemAllocator *phys_mem_alloc, GstMemory *parent, gsize maxsize, GstMemoryFlags flags, gsize align, gsize offset, gsize size)
{
	GstTestPhysMemory *phys_mem;
	phys_mem = g_slice_alloc(sizeof(GstTestPhysMemory));
	if (phys_mem == NULL)
	{
		GST_ERROR_OBJECT(phys_mem_alloc, "could not allocate memory for physmem structure");
		return NULL;
	}

	phys_mem->mapped_virt_addr = NULL;
	phys_mem->phys_addr = 0;
	phys_mem->mapping_refcount = 0;
	phys_mem->internal = NULL;

	//Initializes a newly allocated mem with the given parameters. 
	gst_memory_init(GST_MEMORY_CAST(phys_mem), flags, GST_ALLOCATOR_CAST(phys_mem_alloc), parent, maxsize, align, offset, size);

	return phys_mem;
}


static GstTestPhysMemory* gst_test_phys_mem_allocator_alloc_internal(GstAllocator *allocator, GstMemory *parent, gsize maxsize, GstMemoryFlags flags, gsize align, gsize offset, gsize size)
{
	GstTestPhysMemAllocator *phys_mem_alloc;
	GstTestPhysMemAllocatorClass *klass;
	GstTestPhysMemory *phys_mem;

	phys_mem_alloc = GST_TEST_PHYS_MEM_ALLOCATOR(allocator);
	klass = GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(G_OBJECT_GET_CLASS(allocator));

	//Output a debugging message belonging to the given object in the default category.
	GST_DEBUG_OBJECT(
		allocator,
		"alloc_internal called: maxsize: %u, align: %u, offset: %u, size: %u",
		maxsize,
		align,
		offset,
		size
	);

	phys_mem = gst_test_phys_mem_new_internal(phys_mem_alloc, parent, maxsize, flags, align, offset, size);
	if (phys_mem == NULL)
	{
		GST_WARNING_OBJECT(phys_mem_alloc, "could not create new physmem structure");
		return NULL;
	}

	if (!klass->alloc_phys_mem(phys_mem_alloc, phys_mem, maxsize))
	{
		g_slice_free1(sizeof(GstTestPhysMemory), phys_mem);
		return NULL;
	}

	//GST_MEMORY_FLAG_ZERO_PREFIXED the memory prefix is filled with 0 bytes
	if ((offset > 0) && (flags & GST_MEMORY_FLAG_ZERO_PREFIXED))
	{
		gpointer ptr = klass->map_phys_mem(phys_mem_alloc, phys_mem, maxsize, GST_MAP_WRITE);
		memset(ptr, 0, offset);
		klass->unmap_phys_mem(phys_mem_alloc, phys_mem);
	}

	return phys_mem;
}

//GstAllocationParams *params to control the allocation of memory
static GstMemory* gst_test_phys_mem_allocator_alloc(GstAllocator *allocator, gsize size, GstAllocationParams *params)
{
	gsize maxsize;
	GstTestPhysMemory *phys_mem;

	maxsize = size + params->prefix + params->padding;
	phys_mem = gst_test_phys_mem_allocator_alloc_internal(allocator, NULL, maxsize, params->flags, params->align, params->prefix, size);

	if (phys_mem != NULL)
		GST_INFO_OBJECT(allocator, "allocated memory block %p at phys addr %" GST_TEST_PHYS_ADDR_FORMAT " with %u bytes", (gpointer)phys_mem, phys_mem->phys_addr, size);
	else
		GST_WARNING_OBJECT(allocator, "could not allocate memory block with %u bytes", size);

	return (GstMemory *)phys_mem;
}


static void gst_test_phys_mem_allocator_free(GstAllocator *allocator, GstMemory *memory)
{
	GstTestPhysMemory *phys_mem = (GstTestPhysMemory *)memory;
	GstTestPhysMemAllocator *phys_mem_alloc = GST_TEST_PHYS_MEM_ALLOCATOR(allocator);
	GstTestPhysMemAllocatorClass *klass = GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(G_OBJECT_GET_CLASS(allocator));

	// gst_test_vpu_dec_free_phys_mem()
	klass->free_phys_mem(phys_mem_alloc, phys_mem);

	GST_INFO_OBJECT(allocator, "freed block %p at phys addr %" GST_TEST_PHYS_ADDR_FORMAT " with size: %u", (gpointer)memory, phys_mem->phys_addr, memory->size);

	g_slice_free1(sizeof(GstTestPhysMemory), phys_mem);
}


static gpointer gst_test_phys_mem_allocator_map(GstMemory *mem, gsize maxsize, GstMapFlags flags)
{
	GstTestPhysMemory *phys_mem = (GstTestPhysMemory *)mem;
	GstTestPhysMemAllocator *phys_mem_alloc = GST_TEST_PHYS_MEM_ALLOCATOR(mem->allocator);
	GstTestPhysMemAllocatorClass *klass = GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(G_OBJECT_GET_CLASS(mem->allocator));

	GST_LOG_OBJECT(phys_mem_alloc, "mapping %u bytes from memory block %p (phys addr %" GST_TEST_PHYS_ADDR_FORMAT "), current mapping refcount = %ld -> %ld", maxsize, (gpointer)mem, phys_mem->phys_addr, phys_mem->mapping_refcount, phys_mem->mapping_refcount + 1);

	phys_mem->mapping_refcount++;

	/* In GStreamer, it is not possible to map the same buffer several times
	 * with different flags. Therefore, it is safe to use refcounting here,
	 * since the value of "flags" will be the same with multiple map calls. */

	// there map_phs_mem() is gst_test_vpu_dec_map_phys_mem() 
	// 返回的实际是虚拟地址
	if (phys_mem->mapping_refcount == 1)
	{
		phys_mem->mapping_flags = flags;
		return klass->map_phys_mem(phys_mem_alloc, phys_mem, maxsize, flags);
	}
	else
	{
		g_assert(phys_mem->mapping_flags == flags);
		return phys_mem->mapped_virt_addr;
	}
}


static void gst_test_phys_mem_allocator_unmap(GstMemory *mem)
{
	GstTestPhysMemory *phys_mem = (GstTestPhysMemory *)mem;
	GstTestPhysMemAllocator *phys_mem_alloc = GST_TEST_PHYS_MEM_ALLOCATOR(mem->allocator);
	GstTestPhysMemAllocatorClass *klass = GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(G_OBJECT_GET_CLASS(mem->allocator));

	GST_LOG_OBJECT(phys_mem_alloc, "unmapping memory block %p (phys addr %" GST_TEST_PHYS_ADDR_FORMAT "), current mapping refcount = %ld -> %ld", (gpointer)mem, phys_mem->phys_addr, phys_mem->mapping_refcount, (phys_mem->mapping_refcount > 0) ? (phys_mem->mapping_refcount - 1) : 0);

	// gst_test_vpu_dec_unmap_phys_mem() 
	if (phys_mem->mapping_refcount > 0)
	{
		phys_mem->mapping_refcount--;
		if (phys_mem->mapping_refcount == 0)
			klass->unmap_phys_mem(phys_mem_alloc, phys_mem);
	}
}


static GstMemory* gst_test_phys_mem_allocator_copy(GstMemory *mem, gssize offset, gssize size)
{
	GstTestPhysMemory *copy;
	GstTestPhysMemAllocator *phys_mem_alloc = (GstTestPhysMemAllocator*)(mem->allocator);

	if (size == -1)
		size = ((gssize)(mem->size) > offset) ? (mem->size - offset) : 0;

	copy = gst_test_phys_mem_allocator_alloc_internal(mem->allocator, NULL, mem->maxsize, 0, mem->align, mem->offset + offset, size);
	if (copy == NULL)
	{
		GST_ERROR_OBJECT(phys_mem_alloc, "could not copy memory block - allocation failed");
		return NULL;
	}

	{
		gpointer srcptr, destptr;
		GstTestPhysMemAllocatorClass *klass = GST_TEST_PHYS_MEM_ALLOCATOR_CLASS(G_OBJECT_GET_CLASS(mem->allocator));

		srcptr = klass->map_phys_mem(phys_mem_alloc, (GstTestPhysMemory *)mem, mem->maxsize, GST_MAP_READ);
		destptr = klass->map_phys_mem(phys_mem_alloc, copy, mem->maxsize, GST_MAP_WRITE);

		memcpy(destptr, srcptr, mem->maxsize);

		klass->unmap_phys_mem(phys_mem_alloc, copy);
		klass->unmap_phys_mem(phys_mem_alloc, (GstTestPhysMemory *)mem);
	}

	GST_INFO_OBJECT(
		mem->allocator,
		"copied block %p, new copied block %p; offset: %d, size: %d; source block maxsize: %u, align: %u, offset: %u, size: %u",
		(gpointer)mem, (gpointer)copy,
		offset,
		size,
		mem->maxsize,
		mem->align,
		mem->offset,
		mem->size
	);

	return (GstMemory *)copy;
}


static GstMemory* gst_test_phys_mem_allocator_share(GstMemory *mem, gssize offset, gssize size)
{
	GstTestPhysMemory *phys_mem;
	GstTestPhysMemory *sub;
	GstMemory *parent;

	phys_mem = (GstTestPhysMemory *)mem;

	if (size == -1)
		size = ((gssize)(phys_mem->mem.size) > offset) ? (phys_mem->mem.size - offset) : 0;

	if ((parent = phys_mem->mem.parent) == NULL)
		parent = (GstMemory *)mem;

	sub = gst_test_phys_mem_new_internal(
		GST_TEST_PHYS_MEM_ALLOCATOR(phys_mem->mem.allocator),
		parent,
		phys_mem->mem.maxsize,
		GST_MINI_OBJECT_FLAGS(parent) | GST_MINI_OBJECT_FLAG_LOCK_READONLY,
		phys_mem->mem.align,
		phys_mem->mem.offset + offset,
		size
	);
	if (sub == NULL)
	{
		GST_ERROR_OBJECT(mem->allocator, "could not create new physmem substructure");
		return NULL;
	}

	/* not copying mapped virt addr or mapping ref count, since
	 * mapping is individual to all buffers */
	sub->phys_addr = phys_mem->phys_addr;
	sub->internal = phys_mem->internal;

	GST_INFO_OBJECT(
		mem->allocator,
		"shared block %p, new sub block %p; offset: %d, size: %d; source block maxsize: %u, align: %u, offset: %u, size: %u",
		(gpointer)mem, (gpointer)sub,
		offset,
		size,
		mem->maxsize,
		mem->align,
		mem->offset,
		mem->size
	);

	return (GstMemory *)sub;
}


static gboolean gst_test_phys_mem_allocator_is_span(G_GNUC_UNUSED GstMemory *mem1, G_GNUC_UNUSED GstMemory *mem2, G_GNUC_UNUSED gsize *offset)
{
	return FALSE;
}


guintptr gst_test_phys_memory_get_phys_addr(GstMemory *mem)
{
	return ((GstTestPhysMemory *)mem)->phys_addr;
}


gboolean gst_test_is_phys_memory(GstMemory *mem)
{
	return GST_IS_TEST_PHYS_MEM_ALLOCATOR(mem->allocator);
}


