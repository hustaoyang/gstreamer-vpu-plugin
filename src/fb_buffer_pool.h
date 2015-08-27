#ifndef GST_TEST_VPU_FB_BUFFER_POOL_H
#define GST_TEST_VPU_FB_BUFFER_POOL_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include "framebuffers.h"


G_BEGIN_DECLS


typedef struct _GstTestVpuFbBufferPool GstTestVpuFbBufferPool;
typedef struct _GstTestVpuFbBufferPoolClass GstTestVpuFbBufferPoolClass;


#define GST_TYPE_TEST_VPU_FB_BUFFER_POOL             (gst_test_vpu_fb_buffer_pool_get_type())
#define GST_TEST_VPU_FB_BUFFER_POOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TEST_VPU_FB_BUFFER_POOL, GstTestVpuFbBufferPool))
#define GST_TEST_VPU_FB_BUFFER_POOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TEST_VPU_FB_BUFFER_POOL, GstTestVpuFbBufferPoolClass))


#define GST_BUFFER_POOL_OPTION_TEST_VPU_FRAMEBUFFER "GstBufferPoolOptionTestVpuFramebuffer"


struct _GstTestVpuFbBufferPool
{
	GstBufferPool bufferpool;

	GstTestVpuFramebuffers *framebuffers;
	GstVideoInfo video_info;
	gboolean add_videometa;
};


struct _GstTestVpuFbBufferPoolClass
{
	GstBufferPoolClass parent_class;
};


G_END_DECLS


GType gst_test_vpu_fb_buffer_pool_get_type(void);

/* Note that this function returns a floating reference. See gst_object_ref_sink() for details. */
GstBufferPool *gst_test_vpu_fb_buffer_pool_new(GstTestVpuFramebuffers *framebuffers);

void gst_test_vpu_fb_buffer_pool_set_framebuffers(GstBufferPool *pool, GstTestVpuFramebuffers *framebuffers);

gboolean gst_test_vpu_set_buffer_contents(GstBuffer *buffer, GstTestVpuFramebuffers *framebuffers, VpuFrameBuffer *framebuffer);
void gst_test_vpu_mark_buf_as_not_displayed(GstBuffer *buffer);


#endif

