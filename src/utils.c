#include <gst/gst.h>
#include "utils.h"

// TODO: create one strerror function for the decoder, one for the encoder
gchar const *gst_test_vpu_strerror(VpuDecRetCode code)
{
	switch (code)
	{
		case VPU_DEC_RET_SUCCESS: return "success";
		case VPU_DEC_RET_FAILURE: return "failure";
		case VPU_DEC_RET_INVALID_PARAM: return "invalid param";
		case VPU_DEC_RET_INVALID_HANDLE: return "invalid handle";
		case VPU_DEC_RET_INVALID_FRAME_BUFFER: return "invalid frame buffer";
		case VPU_DEC_RET_INSUFFICIENT_FRAME_BUFFERS: return "insufficient frame buffers";
		case VPU_DEC_RET_INVALID_STRIDE: return "invalid stride";
		case VPU_DEC_RET_WRONG_CALL_SEQUENCE: return "wrong call sequence";
		case VPU_DEC_RET_FAILURE_TIMEOUT: return "failure timeout";
		default:
			return NULL;
	}
}

