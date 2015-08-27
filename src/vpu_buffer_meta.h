#ifndef GST_TEST_VPU_BUFFER_META_H
#define GST_TEST_VPU_BUFFER_META_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include "vpu_wrapper.h"


G_BEGIN_DECLS


typedef struct _GstTestVpuBufferMeta GstTestVpuBufferMeta;


#define GST_TEST_VPU_BUFFER_META_GET(buffer)      ((GstTestVpuBufferMeta *)gst_buffer_get_meta((buffer), gst_test_vpu_buffer_meta_api_get_type()))
#define GST_TEST_VPU_BUFFER_META_ADD(buffer)      (gst_buffer_add_meta((buffer), gst_test_vpu_buffer_meta_get_info(), NULL))
#define GST_TEST_VPU_BUFFER_META_DEL(buffer)      (gst_buffer_remove_meta((buffer), gst_buffer_get_meta((buffer), gst_test_vpu_buffer_meta_api_get_type())))


struct _GstTestVpuBufferMeta
{
	GstMeta meta;

	VpuFrameBuffer *framebuffer;
	gboolean not_displayed_yet;
};


GType gst_test_vpu_buffer_meta_api_get_type(void);
GstMetaInfo const * gst_test_vpu_buffer_meta_get_info(void);


G_END_DECLS


#endif


