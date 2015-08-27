#include <config.h>
#include <gst/gst.h>
#include "decoder/decoder.h"



static gboolean plugin_init(GstPlugin *plugin)
{
	gboolean ret = TRUE;
	ret = ret && gst_element_register(plugin, "testvpudec", GST_RANK_PRIMARY + 1, gst_test_vpu_dec_get_type());
	
	return ret;
}



GST_PLUGIN_DEFINE(
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	testvpu,
	"video en- and decoder elements using the  VPU",
	plugin_init,
	VERSION,
	"LGPL",
	"testvpudec",
	"www.gstreamer.com"
)

