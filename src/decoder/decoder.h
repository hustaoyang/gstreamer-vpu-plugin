#ifndef GST_TEST_VPU_DECODER_H
#define GST_TEST_VPU_DECODER_H

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideodecoder.h>

#include "vpu_wrapper.h"

#include "../framebuffers.h"


G_BEGIN_DECLS


typedef struct _GstTestVpuDec GstTestVpuDec;
typedef struct _GstTestVpuDecClass GstTestVpuDecClass;


#define GST_TYPE_TEST_VPU_DEC             (gst_test_vpu_dec_get_type())
#define GST_TEST_VPU_DEC(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TEST_VPU_DEC, GstTestVpuDec))
#define GST_TEST_VPU_DEC_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TEST_VPU_DEC, GstTestVpuDecClass))
#define GST_IS_TEST_VPU_DEC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TEST_VPU_DEC))
#define GST_IS_TEST_VPU_DEC_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TEST_VPU_DEC))


struct _GstTestVpuDec
{
	GstVideoDecoder parent;

	VpuDecHandle handle;

	VpuDecInitInfo init_info;
	VpuMemInfo mem_info;

	gboolean vpu_inst_opened, is_mjpeg, use_vpuwrapper_flush_call;
	VpuCodStd codec_format;

	GstBuffer *codec_data;

	GstAllocator *allocator;


	// VPU 解码出的 framebuffer， 
	GstTestVpuFramebuffers *current_framebuffers;
	
	// VPU 分配的最小 framebuffers 大小， 一般设为6 才能组成一张完整的图片
	// 
	guint num_additional_framebuffers;
	/* if true, the number of available framebuffers will be recalculated
	 * after the next VPU_DecDecodeBuf() call ; this value is true after the
	 * reset() vfunc is called (not to be confused with VPU_DecReset() ) */
	//调用VPU_DecDecodeBuf() 计算出的实际可用帧， 
	// 
	gboolean recalculate_num_avail_framebuffers;
	/* if true, it means VPU_DecDecodeBuf() will never return the
	 * VPU_DEC_ONE_FRM_CONSUMED output flag, and therefore, consumed frame info
	 * cannot be used for associating input and output frames */
	gboolean no_explicit_frame_boundary;

	gint last_sys_frame_number;
	gboolean delay_sys_frame_numbers;

	GstVideoCodecState *current_output_state;

	GSList *virt_dec_mem_blocks, *phys_dec_mem_blocks;

	GHashTable *frame_table;
};


struct _GstTestVpuDecClass
{
	GstVideoDecoderClass parent_class;
};


GType gst_test_vpu_dec_get_type(void);

gboolean gst_test_vpu_dec_load(void);
void gst_test_vpu_dec_unload(void);


G_END_DECLS


#endif

