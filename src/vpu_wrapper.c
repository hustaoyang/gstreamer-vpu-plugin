#include "vpu_wrapper.h"

VpuDecRetCode VPU_DecLoad()
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetVersionInfo(VpuVersionInfo * pOutVerInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetWrapperVersionInfo(VpuWrapperVersionInfo * pOutVerInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecQueryMem(VpuMemInfo* pOutMemInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecOpen(VpuDecHandle *pOutHandle, VpuDecOpenParam * pInParam,VpuMemInfo* pInMemInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

//未使用
VpuDecRetCode VPU_DecGetCapability(VpuDecHandle InHandle,VpuDecCapability eInCapability, int* pOutCapbility)
{
	return VPU_DEC_RET_SUCCESS;
}

//未使用
VpuDecRetCode VPU_DecDisCapability(VpuDecHandle InHandle,VpuDecCapability eInCapability)
{
	return VPU_DEC_RET_SUCCESS;
}

//VpuDecRetCode VPU_DecSeqInit(VpuDecHandle InHandle, VpuBufferNode* pInData, VpuSeqInfo * pOutInfo);
VpuDecRetCode VPU_DecConfig(VpuDecHandle InHandle, VpuDecConfig InDecConf, void* pInParam)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecDecodeBuf(VpuDecHandle InHandle, VpuBufferNode* pInData,int* pOutBufRetCode)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetInitialInfo(VpuDecHandle InHandle, VpuDecInitInfo * pOutInitInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecRegisterFrameBuffer(VpuDecHandle InHandle,VpuFrameBuffer *pInFrameBufArray, int nNum)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetOutputFrame(VpuDecHandle InHandle, VpuDecOutFrameInfo * pOutFrameInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetConsumedFrameInfo(VpuDecHandle InHandle,VpuDecFrameLengthInfo* pOutFrameInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecOutFrameDisplayed(VpuDecHandle InHandle, VpuFrameBuffer* pInFrameBuf)
{
	return VPU_DEC_RET_SUCCESS;
}

//VpuDecRetCode VPU_DecFlushLeftStream(VpuDecHandle InHandle);
//VpuDecRetCode VPU_DecFlushLeftFrame(VpuDecHandle InHandle);
VpuDecRetCode VPU_DecFlushAll(VpuDecHandle InHandle)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecAllRegFrameInfo(VpuDecHandle InHandle, VpuFrameBuffer** ppOutFrameBuf, int* pOutNum)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecClose(VpuDecHandle InHandle)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecUnLoad()
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecReset(VpuDecHandle InHandle);
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetErrInfo(VpuDecHandle InHandle,VpuDecErrInfo* pErrInfo)
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecGetMem(VpuMemDesc* pInOutMem);
{
	return VPU_DEC_RET_SUCCESS;
}

VpuDecRetCode VPU_DecFreeMem(VpuMemDesc* pInMem)
{
	return VPU_DEC_RET_SUCCESS;
}