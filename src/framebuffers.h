#ifndef GST_TEST_VPU_FRAMEBUFFERS_H
#define GST_TEST_VPU_FRAMEBUFFERS_H

#include <glib.h>
#include <gst/gst.h>
#include "vpu_wrapper.h"


G_BEGIN_DECLS


typedef struct _GstTestVpuFramebuffers GstTestVpuFramebuffers;
typedef struct _GstTestVpuFramebuffersClass GstTestVpuFramebuffersClass;


#define GST_TYPE_TEST_VPU_FRAMEBUFFERS             (gst_test_vpu_framebuffers_get_type())
#define GST_TEST_VPU_FRAMEBUFFERS(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TEST_VPU_FRAMEBUFFERS, GstTestVpuFramebuffers))
#define GST_TEST_VPU_FRAMEBUFFERS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TEST_VPU_FRAMEBUFFERS, GstTestVpuFramebuffersClass))
#define GST_TEST_VPU_FRAMEBUFFERS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_TEST_VPU_FRAMEBUFFERS, GstTestVpuFramebuffersClass))
#define GST_IS_TEST_VPU_FRAMEBUFFERS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TEST_VPU_FRAMEBUFFERS))
#define GST_IS_TEST_VPU_FRAMEBUFFERS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TEST_VPU_FRAMEBUFFERS))

#define GST_TEST_VPU_MIN_NUM_FREE_FRAMEBUFFERS 6


typedef enum
{
	GST_TEST_VPU_FRAMEBUFFERS_UNREGISTERED,
	GST_TEST_VPU_FRAMEBUFFERS_DECODER_REGISTERED,
	GST_TEST_VPU_FRAMEBUFFERS_ENCODER_REGISTERED
} GstTestVpuFramebuffersRegistrationState;


typedef union
{
	struct
	{
		VpuDecHandle handle;
		gboolean decoder_open;
	} dec;
	
	struct
	{
		VpuEncHandle handle;
		gboolean encoder_open;
	} enc;
}
GstTestVpuFramebuffersDecStates;


struct _GstTestVpuFramebuffers
{
	GstObject parent;

	GstTestVpuFramebuffersDecStates decenc_states;

	GstTestVpuFramebuffersRegistrationState registration_state;

	GstAllocator *allocator;

	VpuFrameBuffer *framebuffers;
	guint num_framebuffers;
	gint num_available_framebuffers, decremented_availbuf_counter, num_framebuffers_in_buffers;
	GSList *fb_mem_blocks;
	GMutex available_fb_mutex;
	GCond cond;
	gboolean flushing, exit_loop;

	int y_stride, uv_stride;
	int y_size, u_size, v_size, mv_size;
	int total_size;

	guint pic_width, pic_height;
};


struct _GstTestVpuFramebuffersClass
{
	GstObjectClass parent_class;
};


typedef struct
{
	gint
		pic_width,
		pic_height,
		min_framebuffer_count,
		mjpeg_source_format,
		interlace,
		address_alignment;
}
GstTestVpuFramebufferParams;


#define GST_TEST_VPU_FRAMEBUFFERS_LOCK(framebuffers)   (g_mutex_lock(&(((GstTestVpuFramebuffers*)(framebuffers))->available_fb_mutex)))
#define GST_TEST_VPU_FRAMEBUFFERS_UNLOCK(framebuffers) (g_mutex_unlock(&(((GstTestVpuFramebuffers*)(framebuffers))->available_fb_mutex)))


GType gst_test_vpu_framebuffers_get_type(void);

/* Note that this function returns a floating reference. See gst_object_ref_sink() for details. */
GstTestVpuFramebuffers * gst_test_vpu_framebuffers_new(GstTestVpuFramebufferParams *params, GstAllocator *allocator);

gboolean gst_test_vpu_framebuffers_register_with_decoder(GstTestVpuFramebuffers *framebuffers, VpuDecHandle handle);

void gst_test_vpu_framebuffers_dec_init_info_to_params(VpuDecInitInfo *init_info, GstTestVpuFramebufferParams *params);


/* NOTE: the three functions below must be called with a lock held on framebuffers! */
void gst_test_vpu_framebuffers_set_flushing(GstTestVpuFramebuffers *framebuffers, gboolean flushing);
void gst_test_vpu_framebuffers_wait_until_frames_available(GstTestVpuFramebuffers *framebuffers);
void gst_test_vpu_framebuffers_exit_wait_loop(GstTestVpuFramebuffers *framebuffers);


G_END_DECLS


#endif

