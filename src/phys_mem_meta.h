#ifndef GST_TEST_COMMON_PHYS_MEM_META_H
#define GST_TEST_COMMON_PHYS_MEM_META_H

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TEST_PHYS_ADDR_FORMAT "#lx"
typedef unsigned long gst_test_phys_addr_t;

typedef struct _GstTestPhysMemMeta GstTestPhysMemMeta;


#define GST_TEST_PHYS_MEM_META_GET(buffer)      ((GstTestPhysMemMeta *)gst_buffer_get_meta((buffer), gst_test_phys_mem_meta_api_get_type()))
#define GST_TEST_PHYS_MEM_META_ADD(buffer)      ((GstTestPhysMemMeta *)gst_buffer_add_meta((buffer), gst_test_phys_mem_meta_get_info(), NULL))
#define GST_TEST_PHYS_MEM_META_DEL(buffer)      (gst_buffer_remove_meta((buffer), gst_buffer_get_meta((buffer), gst_test_phys_mem_meta_api_get_type())))


#define GST_BUFFER_POOL_OPTION_TEST_PHYS_MEM "GstBufferPoolOptionTestPhysMem"


struct _GstTestPhysMemMeta
{
	GstMeta meta;

	gst_test_phys_addr_t phys_addr;
	gsize x_padding, y_padding;
	GstBuffer *parent;
};


GType gst_test_phys_mem_meta_api_get_type(void);
GstMetaInfo const * gst_test_phys_mem_meta_get_info(void);


G_END_DECLS


#endif