#include "vpu_buffer_meta.h"


static gboolean gst_test_vpu_buffer_meta_init(GstMeta *meta, G_GNUC_UNUSED gpointer params, G_GNUC_UNUSED GstBuffer *buffer)
{
	GstTestVpuBufferMeta *test_vpu_meta = (GstTestVpuBufferMeta *)meta;
	test_vpu_meta->framebuffer = NULL;
	test_vpu_meta->not_displayed_yet = FALSE;
	return TRUE;
}


static void gst_test_vpu_buffer_meta_free(GstMeta *meta, G_GNUC_UNUSED GstBuffer *buffer)
{
	GstTestVpuBufferMeta *test_vpu_meta = (GstTestVpuBufferMeta *)meta;
	test_vpu_meta->framebuffer = NULL;
}


GType gst_test_vpu_buffer_meta_api_get_type(void)
{
	static volatile GType type;
	static gchar const *tags[] = { "test_vpu", NULL };

	if (g_once_init_enter(&type))
	{
		GType _type = gst_meta_api_type_register("GstTestVpuBufferMetaAPI", tags);
		g_once_init_leave(&type, _type);
	}

	return type;
}


GstMetaInfo const * gst_test_vpu_buffer_meta_get_info(void)
{
	static GstMetaInfo const *meta_buffer_test_vpu_info = NULL;

	if (g_once_init_enter(&meta_buffer_test_vpu_info))
	{
		GstMetaInfo const *meta = gst_meta_register(
			gst_test_vpu_buffer_meta_api_get_type(),
			"GstTestVpuBufferMeta",
			sizeof(GstTestVpuBufferMeta),
			GST_DEBUG_FUNCPTR(gst_test_vpu_buffer_meta_init),
			GST_DEBUG_FUNCPTR(gst_test_vpu_buffer_meta_free),
			(GstMetaTransformFunction)NULL
		);
		g_once_init_leave(&meta_buffer_test_vpu_info, meta);
	}

	return meta_buffer_test_vpu_info;
}

