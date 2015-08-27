#include <string.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include "phys_mem_allocator.h"
#include "framebuffers.h"
#include "utils.h"
#include "mem_blocks.h"


GST_DEBUG_CATEGORY_STATIC(test_vpu_framebuffers_debug);
#define GST_CAT_DEFAULT test_vpu_framebuffers_debug


#define ALIGN_VAL_TO(LENGTH, ALIGN_SIZE)  ( ((guintptr)((LENGTH) + (ALIGN_SIZE) - 1) / (ALIGN_SIZE)) * (ALIGN_SIZE) )
#define FRAME_ALIGN 16


G_DEFINE_TYPE(GstTestVpuFramebuffers, gst_test_vpu_framebuffers, GST_TYPE_OBJECT)


static gboolean gst_test_vpu_framebuffers_configure(GstTestVpuFramebuffers *framebuffers, GstTestVpuFramebufferParams *params, GstAllocator *allocator);
static void gst_test_vpu_framebuffers_finalize(GObject *object);




void gst_test_vpu_framebuffers_class_init(GstTestVpuFramebuffersClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = GST_DEBUG_FUNCPTR(gst_test_vpu_framebuffers_finalize);

	GST_DEBUG_CATEGORY_INIT(test_vpu_framebuffers_debug, "testvpuframebuffers", 0, "VPU framebuffer memory blocks");
}


void gst_test_vpu_framebuffers_init(GstTestVpuFramebuffers *framebuffers)
{
	framebuffers->registration_state = GST_TEST_VPU_FRAMEBUFFERS_UNREGISTERED;
	memset(&(framebuffers->decenc_states), 0, sizeof(GstTestVpuFramebuffersDecStates));

	framebuffers->framebuffers = NULL;
	framebuffers->num_framebuffers = 0;
	framebuffers->num_available_framebuffers = 0;
	framebuffers->decremented_availbuf_counter = 0;
	framebuffers->num_framebuffers_in_buffers = 0;
	framebuffers->fb_mem_blocks = NULL;

	framebuffers->y_stride = framebuffers->uv_stride = 0;
	framebuffers->y_size = framebuffers->u_size = framebuffers->v_size = framebuffers->mv_size = 0;
	framebuffers->total_size = 0;

	framebuffers->flushing = FALSE;
	framebuffers->exit_loop = FALSE;

	g_mutex_init(&(framebuffers->available_fb_mutex));
	g_cond_init(&(framebuffers->cond));
}


GstTestVpuFramebuffers * gst_test_vpu_framebuffers_new(GstTestVpuFramebufferParams *params, GstAllocator *allocator)
{
	GstTestVpuFramebuffers *framebuffers;
	framebuffers = g_object_new(gst_test_vpu_framebuffers_get_type(), NULL);
	if (gst_test_vpu_framebuffers_configure(framebuffers, params, allocator))
		return framebuffers;
	else
		return NULL;
}


gboolean gst_test_vpu_framebuffers_register_with_decoder(GstTestVpuFramebuffers *framebuffers, VpuDecHandle handle)
{
	VpuDecRetCode vpu_ret;

	if (framebuffers->registration_state != GST_TEST_VPU_FRAMEBUFFERS_UNREGISTERED)
	{
		GST_ERROR_OBJECT(framebuffers, "framebuffers already registered");
		return FALSE;
	}

	framebuffers->decenc_states.dec.handle = handle;

	vpu_ret = VPU_DecRegisterFrameBuffer(handle, framebuffers->framebuffers, framebuffers->num_framebuffers);
	if (vpu_ret != VPU_DEC_RET_SUCCESS)
	{
		GST_ERROR_OBJECT(framebuffers, "registering framebuffers failed: %s", gst_test_vpu_strerror(vpu_ret));
		return FALSE;
	}

	framebuffers->registration_state = GST_TEST_VPU_FRAMEBUFFERS_DECODER_REGISTERED;
	framebuffers->decenc_states.dec.decoder_open = TRUE;

	return TRUE;
}


void gst_test_vpu_framebuffers_dec_init_info_to_params(VpuDecInitInfo *init_info, GstTestVpuFramebufferParams *params)
{
	params->pic_width = init_info->nPicWidth;
	params->pic_height = init_info->nPicHeight;
	params->min_framebuffer_count = init_info->nMinFrameBufferCount;
	params->mjpeg_source_format = init_info->nMjpgSourceFormat;
	params->interlace = init_info->nInterlace;
	params->address_alignment = init_info->nAddressAlignment;
}

void gst_test_vpu_framebuffers_set_flushing(GstTestVpuFramebuffers *framebuffers, gboolean flushing)
{
	GST_LOG_OBJECT(framebuffers, "setting flushing value to %d", flushing ? 1 : 0);
	framebuffers->flushing = flushing;
	if (flushing)
		g_cond_signal(&(framebuffers->cond));
}


void gst_test_vpu_framebuffers_wait_until_frames_available(GstTestVpuFramebuffers *framebuffers)
{
	GST_LOG_OBJECT(framebuffers, "flushing = %d  exit_loop = %d", framebuffers->flushing ? 1 : 0, framebuffers->exit_loop ? 1 : 0);
	while ((framebuffers->num_available_framebuffers < GST_TEST_VPU_MIN_NUM_FREE_FRAMEBUFFERS) && !(framebuffers->flushing) && !(framebuffers->exit_loop))
		g_cond_wait(&(framebuffers->cond), &(framebuffers->available_fb_mutex));
	framebuffers->exit_loop = FALSE;
}


void gst_test_vpu_framebuffers_exit_wait_loop(GstTestVpuFramebuffers *framebuffers)
{
	framebuffers->exit_loop = TRUE;
}


static gboolean gst_test_vpu_framebuffers_configure(GstTestVpuFramebuffers *framebuffers, GstTestVpuFramebufferParams *params, GstAllocator *allocator)
{
	int alignment;
	unsigned char *phys_ptr, *virt_ptr;
	guint i;

	g_assert(GST_IS_TEST_PHYS_MEM_ALLOCATOR(allocator));

	framebuffers->num_framebuffers = params->min_framebuffer_count;
	framebuffers->num_available_framebuffers = framebuffers->num_framebuffers;
	framebuffers->decremented_availbuf_counter = 0;
	framebuffers->framebuffers = (VpuFrameBuffer *)g_slice_alloc(sizeof(VpuFrameBuffer) * framebuffers->num_framebuffers);

	framebuffers->allocator = allocator;

	framebuffers->pic_width = ALIGN_VAL_TO(params->pic_width, FRAME_ALIGN);
	if (params->interlace)
		framebuffers->pic_height = ALIGN_VAL_TO(params->pic_height, (2 * FRAME_ALIGN));
	else
		framebuffers->pic_height = ALIGN_VAL_TO(params->pic_height, FRAME_ALIGN);

	framebuffers->y_stride = framebuffers->pic_width;
	framebuffers->y_size = framebuffers->y_stride * framebuffers->pic_height;

	switch (params->mjpeg_source_format)
	{
		case 0: /* I420 (4:2:0) */
			framebuffers->uv_stride = framebuffers->y_stride / 2;
			framebuffers->u_size = framebuffers->v_size = framebuffers->mv_size = framebuffers->y_size / 4;
			break;
		case 1: /* Y42B (4:2:2 horizontal) */
			framebuffers->uv_stride = framebuffers->y_stride / 2;
			framebuffers->u_size = framebuffers->v_size = framebuffers->mv_size = framebuffers->y_size / 2;
			break;
		case 3: /* Y444 (4:4:4) */
			framebuffers->uv_stride = framebuffers->y_stride;
			framebuffers->u_size = framebuffers->v_size = framebuffers->mv_size = framebuffers->y_size;
			break;
		default:
			g_assert_not_reached();
	}

	alignment = params->address_alignment;
	if (alignment > 1)
	{
		framebuffers->y_size = ALIGN_VAL_TO(framebuffers->y_size, alignment);
		framebuffers->u_size = ALIGN_VAL_TO(framebuffers->u_size, alignment);
		framebuffers->v_size = ALIGN_VAL_TO(framebuffers->v_size, alignment);
		framebuffers->mv_size = ALIGN_VAL_TO(framebuffers->mv_size, alignment);
	}

	framebuffers->total_size = framebuffers->y_size + framebuffers->u_size + framebuffers->v_size + framebuffers->mv_size + alignment;
	GST_INFO_OBJECT(
		framebuffers,
		"framebuffer requested width/height: %u/%u  actual width/height (after alignment): %u/%u  Y stride: %u",
		params->pic_width, params->pic_height,
		framebuffers->pic_width, framebuffers->pic_height,
		framebuffers->y_stride
	);
	GST_INFO_OBJECT(
		framebuffers,
		"num framebuffers:  total: %u  available: %d",
		framebuffers->num_framebuffers, framebuffers->num_available_framebuffers
	);
	GST_INFO_OBJECT(
		framebuffers,
		"framebuffer memory block size:  total: %d  Y: %d  U: %d  V: %d  Mv:  %d  alignment: %d",
		framebuffers->total_size, framebuffers->y_size, framebuffers->u_size, framebuffers->v_size, framebuffers->mv_size, alignment
	);
	GST_INFO_OBJECT(
		framebuffers,
		"total memory required for all framebuffers: %d * %d = %d byte",
		framebuffers->total_size, framebuffers->num_framebuffers, framebuffers->total_size * framebuffers->num_framebuffers
	);

	for (i = 0; i < framebuffers->num_framebuffers; ++i)
	{
		GstTestPhysMemory *memory;
		VpuFrameBuffer *framebuffer;

		framebuffer = &(framebuffers->framebuffers[i]);

		memory = (GstTestPhysMemory *)gst_allocator_alloc(allocator, framebuffers->total_size, NULL);
		if (memory == NULL)
			return FALSE;
		gst_test_vpu_append_phys_mem_block(memory, &(framebuffers->fb_mem_blocks));

		phys_ptr = (unsigned char*)(memory->phys_addr);
		virt_ptr = (unsigned char*)(memory->mapped_virt_addr); /* TODO */

		if (alignment > 1)
		{
			phys_ptr = (unsigned char*)ALIGN_VAL_TO(phys_ptr, alignment);
			virt_ptr = (unsigned char*)ALIGN_VAL_TO(virt_ptr, alignment);
		}

		framebuffer->nStrideY = framebuffers->y_stride;
		framebuffer->nStrideC = framebuffers->uv_stride;	

		/* fill phy addr*/
		framebuffer->pbufY     = phys_ptr;
		framebuffer->pbufCb    = phys_ptr + framebuffers->y_size;
		framebuffer->pbufCr    = phys_ptr + framebuffers->y_size + framebuffers->u_size;
		framebuffer->pbufMvCol = phys_ptr + framebuffers->y_size + framebuffers->u_size + framebuffers->v_size;

		/* fill virt addr */
		framebuffer->pbufVirtY     = virt_ptr;
		framebuffer->pbufVirtCb    = virt_ptr + framebuffers->y_size;
		framebuffer->pbufVirtCr    = virt_ptr + framebuffers->y_size + framebuffers->u_size;
		framebuffer->pbufVirtMvCol = virt_ptr + framebuffers->y_size + framebuffers->u_size + framebuffers->v_size;

		framebuffer->pbufY_tilebot = 0;
		framebuffer->pbufCb_tilebot = 0;
		framebuffer->pbufVirtY_tilebot = 0;
		framebuffer->pbufVirtCb_tilebot = 0;
	}

	return TRUE;
}


static void gst_test_vpu_framebuffers_finalize(GObject *object)
{
	GstTestVpuFramebuffers *framebuffers = GST_TEST_VPU_FRAMEBUFFERS(object);

	GST_TEST_VPU_FRAMEBUFFERS_LOCK(framebuffers);
	gst_test_vpu_framebuffers_set_flushing(framebuffers, TRUE);
	g_cond_signal(&(framebuffers->cond));
	GST_TEST_VPU_FRAMEBUFFERS_UNLOCK(framebuffers);

	g_mutex_clear(&(framebuffers->available_fb_mutex));
	g_cond_clear(&(framebuffers->cond));

	GST_INFO_OBJECT(framebuffers, "freeing framebuffer memory");

	if (framebuffers->framebuffers != NULL)
	{
		g_slice_free1(sizeof(VpuFrameBuffer) * framebuffers->num_framebuffers, framebuffers->framebuffers);
		framebuffers->framebuffers = NULL;
	}

	gst_test_vpu_free_phys_mem_blocks((GstTestPhysMemAllocator *)(framebuffers->allocator), &(framebuffers->fb_mem_blocks));

	G_OBJECT_CLASS(gst_test_vpu_framebuffers_parent_class)->finalize(object);
}

