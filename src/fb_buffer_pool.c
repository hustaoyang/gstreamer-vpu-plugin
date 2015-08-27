/*
 // GstBufferPool
    Use the associated macros to access the public variables
  
 // GstBufferPoolClass 
    The GstBufferPool class
	struct GstBufferPoolClass {
	  GstObjectClass    object_class;

	  const gchar ** (*get_options)    (GstBufferPool *pool);
	  gboolean       (*set_config)     (GstBufferPool *pool, GstStructure *config);

	  gboolean       (*start)          (GstBufferPool *pool);
	  gboolean       (*stop)           (GstBufferPool *pool);

	  GstFlowReturn  (*acquire_buffer) (GstBufferPool *pool, GstBuffer **buffer,
										GstBufferPoolAcquireParams *params);
	  GstFlowReturn  (*alloc_buffer)   (GstBufferPool *pool, GstBuffer **buffer,
										GstBufferPoolAcquireParams *params);
	  void           (*reset_buffer)   (GstBufferPool *pool, GstBuffer *buffer);
	  void           (*release_buffer) (GstBufferPool *pool, GstBuffer *buffer);
	  void           (*free_buffer)    (GstBufferPool *pool, GstBuffer *buffer);
	  void           (*flush_start)    (GstBufferPool *pool);
	  void           (*flush_stop)     (GstBufferPool *pool);
	};
 */

#include <string.h>
#include "vpu_wrapper.h"
#include "phys_mem_meta.h"
#include "fb_buffer_pool.h"
#include "utils.h"
#include "vpu_buffer_meta.h"


GST_DEBUG_CATEGORY_STATIC(test_vpu_fb_buffer_pool_debug);
#define GST_CAT_DEFAULT test_vpu_fb_buffer_pool_debug


static void gst_test_vpu_fb_buffer_pool_finalize(GObject *object);
static const gchar ** gst_test_vpu_fb_buffer_pool_get_options(GstBufferPool *pool);
static gboolean gst_test_vpu_fb_buffer_pool_set_config(GstBufferPool *pool, GstStructure *config);
static GstFlowReturn gst_test_vpu_fb_buffer_pool_alloc_buffer(GstBufferPool *pool, GstBuffer **buffer, GstBufferPoolAcquireParams *params);
static void gst_test_vpu_fb_buffer_pool_release_buffer(GstBufferPool *pool, GstBuffer *buffer);


G_DEFINE_TYPE(GstTestVpuFbBufferPool, gst_test_vpu_fb_buffer_pool, GST_TYPE_BUFFER_POOL)




static void gst_test_vpu_fb_buffer_pool_finalize(GObject *object)
{
	GstTestVpuFbBufferPool *vpu_pool = GST_TEST_VPU_FB_BUFFER_POOL(object);

	if (vpu_pool->framebuffers != NULL)
		gst_object_unref(vpu_pool->framebuffers);

	GST_TRACE_OBJECT(vpu_pool, "shutting down buffer pool");

	G_OBJECT_CLASS(gst_test_vpu_fb_buffer_pool_parent_class)->finalize(object);
}


static const gchar ** gst_test_vpu_fb_buffer_pool_get_options(G_GNUC_UNUSED GstBufferPool *pool)
{
	static const gchar *options[] =
	{
		GST_BUFFER_POOL_OPTION_VIDEO_META,
		GST_BUFFER_POOL_OPTION_TEST_VPU_FRAMEBUFFER,
		NULL
	};

	return options;
}


static gboolean gst_test_vpu_fb_buffer_pool_set_config(GstBufferPool *pool, GstStructure *config)
{
	GstTestVpuFbBufferPool *vpu_pool;
	GstVideoInfo info;
	GstCaps *caps;
	guint size;
	guint min, max;

	vpu_pool = GST_TEST_VPU_FB_BUFFER_POOL(pool);

	// Get the configuration values from config 
	if (!gst_buffer_pool_config_get_params(config, &caps, &size, &min, &max))
	{
		GST_ERROR_OBJECT(pool, "pool configuration invalid");
		return FALSE;
	}

	if (caps == NULL)
	{
		GST_ERROR_OBJECT(pool, "configuration contains no caps");
		return FALSE;
	}

	// Parse caps and update info 
	if (!gst_video_info_from_caps(&info, caps))
	{
		GST_ERROR_OBJECT(pool, "caps cannot be parsed for video info");
		return FALSE;
	}

	vpu_pool->video_info = info;

	vpu_pool->video_info.stride[0] = vpu_pool->framebuffers->y_stride;
	vpu_pool->video_info.stride[1] = vpu_pool->framebuffers->uv_stride;
	vpu_pool->video_info.stride[2] = vpu_pool->framebuffers->uv_stride;
	vpu_pool->video_info.offset[0] = 0;
	vpu_pool->video_info.offset[1] = vpu_pool->framebuffers->y_size;
	vpu_pool->video_info.offset[2] = vpu_pool->framebuffers->y_size + vpu_pool->framebuffers->u_size;
	vpu_pool->video_info.size = vpu_pool->framebuffers->total_size;

	// Check if config contains option
	vpu_pool->add_videometa = gst_buffer_pool_config_has_option(config, GST_BUFFER_POOL_OPTION_VIDEO_META);

	return GST_BUFFER_POOL_CLASS(gst_test_vpu_fb_buffer_pool_parent_class)->set_config(pool, config);
}


static GstFlowReturn gst_test_vpu_fb_buffer_pool_alloc_buffer(GstBufferPool *pool, GstBuffer **buffer, G_GNUC_UNUSED GstBufferPoolAcquireParams *params)
{
	GstBuffer *buf;
	GstTestVpuFbBufferPool *vpu_pool;
	GstVideoInfo *info;

	vpu_pool = GST_TEST_VPU_FB_BUFFER_POOL(pool);

	info = &(vpu_pool->video_info);

	buf = gst_buffer_new();
	if (buf == NULL)
	{
		GST_ERROR_OBJECT(pool, "could not create new buffer");
		return GST_FLOW_ERROR;
	}

	// Add metadata for info to buffer using the parameters in params
	GST_TEST_VPU_BUFFER_META_ADD(buf);
	GST_TEST_PHYS_MEM_META_ADD(buf);

	if (vpu_pool->add_videometa)
	{
		gst_buffer_add_video_meta_full(
			buf,
			GST_VIDEO_FRAME_FLAG_NONE,
			GST_VIDEO_INFO_FORMAT(info),
			GST_VIDEO_INFO_WIDTH(info), GST_VIDEO_INFO_HEIGHT(info),
			GST_VIDEO_INFO_N_PLANES(info),
			info->offset,
			info->stride
		);
	}

	*buffer = buf;

	return GST_FLOW_OK;
}


static void gst_test_vpu_fb_buffer_pool_release_buffer(GstBufferPool *pool, GstBuffer *buffer)
{
	GstTestVpuFbBufferPool *vpu_pool;

	vpu_pool = GST_TEST_VPU_FB_BUFFER_POOL(pool);
	g_assert(vpu_pool->framebuffers != NULL);

	if (vpu_pool->framebuffers->registration_state == GST_TEST_VPU_FRAMEBUFFERS_DECODER_REGISTERED)
	{
		VpuDecRetCode dec_ret;
		GstTestVpuBufferMeta *vpu_meta;
		GstTestPhysMemMeta *phys_mem_meta;

		vpu_meta = GST_TEST_VPU_BUFFER_META_GET(buffer);
		phys_mem_meta = GST_TEST_PHYS_MEM_META_GET(buffer);

		GST_TEST_VPU_FRAMEBUFFERS_LOCK(vpu_pool->framebuffers);

		if ((vpu_meta->framebuffer != NULL) && (phys_mem_meta != NULL) && (phys_mem_meta->phys_addr != 0))
		{
			if (vpu_meta->not_displayed_yet && vpu_pool->framebuffers->decenc_states.dec.decoder_open)
			{
				dec_ret = VPU_DecOutFrameDisplayed(vpu_pool->framebuffers->decenc_states.dec.handle, vpu_meta->framebuffer);
				if (dec_ret != VPU_DEC_RET_SUCCESS)
					GST_ERROR_OBJECT(pool, "clearing display framebuffer failed: %s", gst_test_vpu_strerror(dec_ret));
				else
				{
					vpu_meta->not_displayed_yet = FALSE;
					if (vpu_pool->framebuffers->decremented_availbuf_counter > 0)
					{
						vpu_pool->framebuffers->num_available_framebuffers++;
						vpu_pool->framebuffers->decremented_availbuf_counter--;
						vpu_pool->framebuffers->num_framebuffers_in_buffers--;
						GST_LOG_OBJECT(pool, "number of available buffers: %d -> %d", vpu_pool->framebuffers->num_available_framebuffers - 1, vpu_pool->framebuffers->num_available_framebuffers);
					}
					GST_LOG_OBJECT(pool, "cleared buffer %p", (gpointer)buffer);
				}
			}
			else if (!vpu_pool->framebuffers->decenc_states.dec.decoder_open)
				GST_DEBUG_OBJECT(pool, "not clearing buffer %p, since VPU decoder is closed", (gpointer)buffer);
			else
				GST_DEBUG_OBJECT(pool, "buffer %p already cleared", (gpointer)buffer);
		}
		else
		{
			GST_DEBUG_OBJECT(pool, "buffer %p does not contain physical memory and/or a VPU framebuffer pointer, and does not need to be cleared", (gpointer)buffer);
		}

		/* Clear out old memory blocks ; the decoder always fills empty buffers with new memory
		 * blocks when it needs to push a newly decoded frame downstream anyway
		 * (see gst_test_vpu_set_buffer_contents() below)
		 * removing the now-unused memory blocks immediately avoids buildup of unused but
		 * still allocated memory */
		gst_buffer_remove_all_memory(buffer);

		g_cond_signal(&(vpu_pool->framebuffers->cond));

		GST_TEST_VPU_FRAMEBUFFERS_UNLOCK(vpu_pool->framebuffers);
	}

	GST_BUFFER_POOL_CLASS(gst_test_vpu_fb_buffer_pool_parent_class)->release_buffer(pool, buffer);
}


static void gst_test_vpu_fb_buffer_pool_class_init(GstTestVpuFbBufferPoolClass *klass)
{
	GObjectClass *object_class;
	GstBufferPoolClass *parent_class;
	
	object_class = G_OBJECT_CLASS(klass);
	parent_class = GST_BUFFER_POOL_CLASS(klass);

	GST_DEBUG_CATEGORY_INIT(test_vpu_fb_buffer_pool_debug, "testvpufbbufferpool", 0, "VPU framebuffers buffer pool");

	object_class->finalize       = GST_DEBUG_FUNCPTR(gst_test_vpu_fb_buffer_pool_finalize);
	parent_class->get_options    = GST_DEBUG_FUNCPTR(gst_test_vpu_fb_buffer_pool_get_options);
	parent_class->set_config     = GST_DEBUG_FUNCPTR(gst_test_vpu_fb_buffer_pool_set_config);
	parent_class->alloc_buffer   = GST_DEBUG_FUNCPTR(gst_test_vpu_fb_buffer_pool_alloc_buffer);
	parent_class->release_buffer = GST_DEBUG_FUNCPTR(gst_test_vpu_fb_buffer_pool_release_buffer);
}


static void gst_test_vpu_fb_buffer_pool_init(GstTestVpuFbBufferPool *pool)
{
	pool->framebuffers = NULL;
	pool->add_videometa = FALSE;

	GST_INFO_OBJECT(pool, "initializing VPU buffer pool");
}

//·ÖÅäframe buffer pool
GstBufferPool *gst_test_vpu_fb_buffer_pool_new(GstTestVpuFramebuffers *framebuffers)
{
	GstTestVpuFbBufferPool *vpu_pool;

	g_assert(framebuffers != NULL);

	vpu_pool = g_object_new(gst_test_vpu_fb_buffer_pool_get_type(), NULL);
	vpu_pool->framebuffers = gst_object_ref(framebuffers);

	return GST_BUFFER_POOL_CAST(vpu_pool);
}


void gst_test_vpu_fb_buffer_pool_set_framebuffers(GstBufferPool *pool, GstTestVpuFramebuffers *framebuffers)
{
	GstTestVpuFbBufferPool *vpu_pool = GST_TEST_VPU_FB_BUFFER_POOL(pool);

	g_assert(framebuffers != NULL);

	if (framebuffers == vpu_pool->framebuffers)
		return;

	/* it is good practice to first ref the new, then unref the old object
	 * even though the case of identical pointers is caught above */
	gst_object_ref(framebuffers);

	if (vpu_pool->framebuffers != NULL)
		gst_object_unref(vpu_pool->framebuffers);

	vpu_pool->framebuffers = framebuffers;
}


gboolean gst_test_vpu_set_buffer_contents(GstBuffer *buffer, GstTestVpuFramebuffers *framebuffers, VpuFrameBuffer *framebuffer)
{
	GstVideoMeta *video_meta;
	GstTestVpuBufferMeta *vpu_meta;
	GstTestPhysMemMeta *phys_mem_meta;
	GstMemory *memory;

	video_meta = gst_buffer_get_video_meta(buffer);
	if (video_meta == NULL)
	{
		GST_ERROR("buffer with pointer %p has no video metadata", (gpointer)buffer);
		return FALSE;
	}

	vpu_meta = GST_TEST_VPU_BUFFER_META_GET(buffer);
	if (vpu_meta == NULL)
	{
		GST_ERROR("buffer with pointer %p has no VPU metadata", (gpointer)buffer);
		return FALSE;
	}

	phys_mem_meta = GST_TEST_PHYS_MEM_META_GET(buffer);
	if (phys_mem_meta == NULL)
	{
		GST_ERROR("buffer with pointer %p has no phys mem metadata", (gpointer)buffer);
		return FALSE;
	}

	{
		gsize x_padding = 0, y_padding = 0;

		if (framebuffers->pic_width > video_meta->width)
			x_padding = framebuffers->pic_width - video_meta->width;
		if (framebuffers->pic_height > video_meta->height)
			y_padding = framebuffers->pic_height - video_meta->height;

		vpu_meta->framebuffer = framebuffer;

		phys_mem_meta->phys_addr = (guintptr)(framebuffer->pbufY);
		phys_mem_meta->x_padding = x_padding;
		phys_mem_meta->y_padding = y_padding;

		GST_LOG("setting phys mem meta for buffer with pointer %p: phys addr %" GST_TEST_PHYS_ADDR_FORMAT " x/y padding %" G_GSIZE_FORMAT "/%" G_GSIZE_FORMAT, (gpointer)buffer, phys_mem_meta->phys_addr, phys_mem_meta->x_padding, phys_mem_meta->y_padding);

		memory = gst_memory_new_wrapped(
			GST_MEMORY_FLAG_NO_SHARE,
			framebuffer->pbufVirtY,
			framebuffers->total_size,
			0,
			framebuffers->total_size,
			NULL,
			NULL
		);
	}

	GST_TEST_VPU_FRAMEBUFFERS_LOCK(framebuffers);
	framebuffers->num_framebuffers_in_buffers++;
	GST_TEST_VPU_FRAMEBUFFERS_UNLOCK(framebuffers);

	/* remove any existing memory blocks */
	gst_buffer_remove_all_memory(buffer);
	/* and append the new memory block */
	gst_buffer_append_memory(buffer, memory);

	return TRUE;
}


void gst_test_vpu_mark_buf_as_not_displayed(GstBuffer *buffer)
{
	GstTestVpuBufferMeta *vpu_meta = GST_TEST_VPU_BUFFER_META_GET(buffer);
	g_assert(vpu_meta != NULL);
	vpu_meta->not_displayed_yet = TRUE;
}

