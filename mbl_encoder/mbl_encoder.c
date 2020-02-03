#include "mbl_encoder.h"

#include "mbl_encoder_32_16.h"
#include "mbl_screen_32.h"

#include <string.h>

MBL_ENCODER* mbl_createEncoder(int nEncType, unsigned int nScreenWidth, unsigned int nScreenHeight)
{
	MBL_ENCODER* pEnc = (MBL_ENCODER*)malloc(sizeof(MBL_ENCODER));
	if (pEnc)
	{
		memset(pEnc, 0, sizeof(MBL_ENCODER));

		pEnc->nEncodeType		= nEncType;

		pEnc->nScreenWidth		= nScreenWidth;
		pEnc->nScreenHeight		= nScreenHeight;
		pEnc->nScreenWidthP16	= nScreenWidth/16;
		pEnc->nScreenHeightP16	= nScreenHeight/16;
		pEnc->nScreenSquares	= pEnc->nScreenWidthP16 * pEnc->nScreenHeightP16;
		pEnc->nScreenSize32bit	= nScreenWidth * nScreenHeight * 4;

		switch (nEncType & MBL_ENCODE_MASK)
		{
		case MBL_ENCODE_RAW_16BIT:
			{
				pEnc->nPacketSize16bit  = 
					((pEnc->nScreenSquares/252)+1)*512 //headers
					+pEnc->nScreenSquares*512 //data
					+512;//frame end

				mbl_resizebuffer(&pEnc->dScreenResult, nScreenWidth * nScreenHeight * 2);//16bit
			}break;
		}

		pEnc->pScreenChanges    = (unsigned char*)malloc(pEnc->nScreenSquares);
		memset(pEnc->pScreenChanges, 0, pEnc->nScreenSquares);

		mbl_resizebuffer(&pEnc->dBuffer4Encoding, pEnc->nPacketSize16bit);
	}
	return pEnc;
}

void mbl_deleteEncoder(MBL_ENCODER* pEnc)
{
	if (!pEnc) return;

	if (pEnc->dBuffer4Encoding.pBuffer) free(pEnc->dBuffer4Encoding.pBuffer);

	if (pEnc->dScreens[0].pBuffer) free(pEnc->dScreens[0].pBuffer);
	if (pEnc->dScreens[1].pBuffer) free(pEnc->dScreens[1].pBuffer);

	if (pEnc->dScreenResult.pBuffer) free(pEnc->dScreenResult.pBuffer);

	if (pEnc->pScreenChanges) free(pEnc->pScreenChanges);

	free(pEnc);
}

int mbl_encode_from_32bit(MBL_ENCODER* pEnc, unsigned char* pData)
{
	if (!pEnc) return 0;

	switch (pEnc->nEncodeType & MBL_ENCODE_MASK)
	{
	case MBL_ENCODE_RAW_16BIT:
                        mbl_updatescreen_32_to_16(pEnc, pData);
			return _mbl_encode_raw_32bit_2_16bit(pEnc, pData);
	}

	return 0;
}


