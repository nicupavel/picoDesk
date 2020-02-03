#include "mbl_utils.h"

#include <string.h>

unsigned int mbl_resizebuffer(MBL_BUFFER* pBuffer, unsigned int nNewSize)
{
	if (nNewSize == 0)
	{
		if (pBuffer->pBuffer != NULL)
		{
			free(pBuffer->pBuffer);
			pBuffer->pBuffer = NULL;
		}

		pBuffer->nBufferSize     = 0;
		pBuffer->nBufferSizeUsed = 0;
		
		return 0;
	}
	else if (pBuffer->pBuffer == NULL)
	{
		if (pBuffer->nBufferSize >= nNewSize)
		{//the size is big enough - no resize
			return pBuffer->nBufferSize;
		}
		else
		{
			if (pBuffer->pBuffer) free(pBuffer->pBuffer);

			pBuffer->pBuffer = (unsigned char*)malloc(nNewSize);
			if (pBuffer->pBuffer)
			{
				memset(pBuffer->pBuffer, 0, nNewSize);
				pBuffer->nBufferSize     = nNewSize;
				pBuffer->nBufferSizeUsed = nNewSize;
				return nNewSize;
			}
			else
			{
				pBuffer->pBuffer		 = NULL;
				pBuffer->nBufferSize     = 0;
				pBuffer->nBufferSizeUsed = 0;
				return 0;
			}
		}
	}
	return 0;
}
