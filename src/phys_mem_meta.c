#include "phys_mem_meta.h"


GST_DEBUG_CATEGORY_STATIC(test_phys_mem_meta_debug);
#define GST_CAT_DEFAULT test_phys_mem_meta_debug


static gboolean gst_test_phys_mem_meta_init(GstMeta *meta, G_GNUC_UNUSED gpointer params, G_GNUC_UNUSED GstBuffer *buffer)
{
	GstTestPhysMemMeta *test_phys_mem_meta = (GstTestPhysMemMeta *)meta;
	test_phys_mem_meta->phys_addr = 0;
	test_phys_mem_meta->x_padding = 0;
	test_phys_mem_meta->y_padding = 0;
	test_phys_mem_meta->parent = NULL;
	return TRUE;
}


GType gst_test_phys_mem_meta_api_get_type(void)
{
	static volatile GType type;
	static gchar const *tags[] = { "memory", "phys_mem", NULL };

	if (g_once_init_enter(&type))
	{
		GType _type = gst_meta_api_type_register("GstTestPhysMemMetaAPI", tags);
		g_once_init_leave(&type, _type);

		GST_DEBUG_CATEGORY_INIT(test_phys_mem_meta_debug, "testphysmemmeta", 0, "Physical memory metadata");
	}

	return type;
}

static gboolean gst_test_phys_mem_meta_transform(GstBuffer *dest, GstMeta *meta, GstBuffer *buffer, GQuark type, gpointer data)
{
	GstTestPhysMemMeta *dmeta, *smeta;

	smeta = (GstTestPhysMemMeta *)meta;

	if (GST_META_TRANSFORM_IS_COPY(type))
	{
		GstMetaTransformCopy *copy = data;
		gboolean do_copy = FALSE;

		if (!(copy->region)) // TODO: is this check correct?
		{
			GST_LOG("not copying metadata: only a region is being copied (not the entire block)");
		}
		else
		{
			guint n_mem_buffer, n_mem_dest;

			//return the amount of memory block in this buffer
			n_mem_buffer = gst_buffer_n_memory(buffer);
			n_mem_dest = gst_buffer_n_memory(dest);

			/* only copy if both buffers have 1 identical memory */
			if ((n_mem_buffer == n_mem_dest) && (n_mem_dest == 1))
			{
				GstMemory *mem1, *mem2;

				//Get the memory block at index idx in buffer
				mem1 = gst_buffer_get_memory(dest, 0);
				mem2 = gst_buffer_get_memory(buffer, 0);

				if (mem1 == mem2)
				{
					GST_LOG("copying physmem metadata: memory blocks identical");
					do_copy = TRUE;
				}
				else
					GST_LOG("not copying physmem metadata: memory blocks not identical");

				gst_memory_unref(mem1);
				gst_memory_unref(mem2);
			}
			else
				GST_LOG("not copying physmem metadata: num memory blocks in source/dest: %u/%u", n_mem_buffer, n_mem_dest);
		}

		if (do_copy)
		{
			/* only copy if the complete data is copied as well */
			//Add metadata for GstMetaInfo to buffer using the parameters in params
			dmeta = (GstTestPhysMemMeta *)gst_buffer_add_meta(dest, gst_test_phys_mem_meta_get_info(), NULL);

			if (!dmeta)
			{
				GST_ERROR("could not add physmem metadata to the dest buffer");
				return FALSE;
			}

			dmeta->phys_addr = smeta->phys_addr;
			dmeta->x_padding = smeta->x_padding;
			dmeta->y_padding = smeta->y_padding;
			if (smeta->parent)
				dmeta->parent = gst_buffer_ref(smeta->parent);
			else
				dmeta->parent = gst_buffer_ref(buffer);
		}
	}

	return TRUE;
}


static void gst_test_phys_mem_meta_free(GstMeta *meta, G_GNUC_UNUSED GstBuffer *buffer)
{
	GstTestPhysMemMeta *smeta = (GstTestPhysMemMeta *)meta;
	GST_TRACE("freeing physmem metadata with phys addr %" GST_TEST_PHYS_ADDR_FORMAT, smeta->phys_addr);
	gst_buffer_replace(&smeta->parent, NULL);
}


GstMetaInfo const * gst_test_phys_mem_meta_get_info(void)
{
	static GstMetaInfo const *gst_test_phys_mem_meta_info = NULL;

	if (g_once_init_enter(&gst_test_phys_mem_meta_info))
	{
		GstMetaInfo const *meta = gst_meta_register(
			gst_test_phys_mem_meta_api_get_type(),
			"GstTestPhysMemMeta",
			sizeof(GstTestPhysMemMeta),
			GST_DEBUG_FUNCPTR(gst_test_phys_mem_meta_init),
			GST_DEBUG_FUNCPTR(gst_test_phys_mem_meta_free),
			GST_DEBUG_FUNCPTR(gst_test_phys_mem_meta_transform)
		);
		g_once_init_leave(&gst_test_phys_mem_meta_info, meta);
	}

	return gst_test_phys_mem_meta_info;
}

