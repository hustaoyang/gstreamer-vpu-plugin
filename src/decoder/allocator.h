#ifndef GST_TEST_VPU_DECODER_ALLOCATOR_H
#define GST_TEST_VPU_DECODER_ALLOCATOR_H

#include <glib.h>
#include "../phys_mem_allocator.h"


G_BEGIN_DECLS


typedef struct _GstTestVpuDecAllocator GstTestVpuDecAllocator;
typedef struct _GstTestVpuDecAllocatorClass GstTestVpuDecAllocatorClass;
typedef struct _GstTestVpuDecMemory GstTestVpuDecMemory;


#define GST_TYPE_TEST_VPU_DEC_ALLOCATOR             (gst_test_vpu_dec_allocator_get_type())
#define GST_TEST_VPU_DEC_ALLOCATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TEST_VPU_DEC_ALLOCATOR, GstTestVpuDecAllocator))
#define GST_TEST_VPU_DEC_ALLOCATOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TEST_VPU_DEC_ALLOCATOR, GstTestVpuDecAllocatorClass))
#define GST_IS_TEST_VPU_DEC_ALLOCATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TEST_VPU_DEC_ALLOCATOR))
#define GST_IS_TEST_VPU_DEC_ALLOCATOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TEST_VPU_DEC_ALLOCATOR))

#define GST_TEST_VPU_DEC_ALLOCATOR_MEM_TYPE "TestVpuDecMemory"


struct _GstTestVpuDecAllocator
{
	GstTestPhysMemAllocator parent;
};


struct _GstTestVpuDecAllocatorClass
{
	GstTestPhysMemAllocatorClass parent_class;
};


GType gst_test_vpu_dec_allocator_get_type(void);

/* Note that this function returns a floating reference. See gst_object_ref_sink() for details. */
GstAllocator* gst_test_vpu_dec_allocator_new(void);


G_END_DECLS


#endif


