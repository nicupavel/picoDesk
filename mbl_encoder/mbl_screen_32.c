#include "mbl_screen_32.h"
#include "mbl_utils.h"

#include <string.h>

#define _BPP 4

void mbl_updatescreen_32_to_16(MBL_ENCODER* pEnc, unsigned char* pData)
{
	if ((pEnc->nEncodeType & MBL_ENCODE_CHANGEONLY)==0)
	{//send everything, no double buffer is needed
		memset(pEnc->pScreenChanges, 1, pEnc->nScreenSquares);

		unsigned char r,g,b;
		unsigned int t,tbpp,t2;
		for (unsigned int sor=0;sor<pEnc->nScreenHeight;sor++)
			for (unsigned int osz=0;osz<pEnc->nScreenWidth;osz++)
			{
				t	 = sor*pEnc->nScreenWidth+osz;
				tbpp = t*_BPP;
				t2   = t<<1;//*2

				b = (pData[tbpp  ]) >> 3 ;		
				g = (pData[tbpp+1]) >> 2;
				r = (pData[tbpp+2]) >> 3;

				pEnc->dScreenResult.pBuffer[t2+1] = (r<<3) | ((g >> 3)&0x07);
				pEnc->dScreenResult.pBuffer[t2]   = (b)    | ((g << 5)&0xE0);
			}
	}
	else
	{//send only changes, double buffer is needed
		MBL_BUFFER* newScreen;
		MBL_BUFFER* oldScreen;
		if (pEnc->nScreenActive == 0)
		{
			newScreen = &pEnc->dScreens[0];
			oldScreen = &pEnc->dScreens[1];

			pEnc->nScreenActive = 1;
		}
		else
		{
			newScreen = &pEnc->dScreens[1];
			oldScreen = &pEnc->dScreens[0];

			pEnc->nScreenActive = 0;
		}

		mbl_resizebuffer(newScreen, pEnc->nScreenSize32bit);
		mbl_resizebuffer(oldScreen, pEnc->nScreenSize32bit);

		memcpy(newScreen->pBuffer, pData, pEnc->nScreenSize32bit);
		memset(pEnc->pScreenChanges, 0, pEnc->nScreenSquares);

		unsigned char r,g,b;
		unsigned int plane;
		unsigned int t,tbpp,t2;

		for (unsigned int sor=0;sor<pEnc->nScreenHeight;sor++)
			for (unsigned int osz=0;osz<pEnc->nScreenWidth;osz++)
			{
				t	 = sor*pEnc->nScreenWidth+osz;
				tbpp = t<<2;//*_BPP;
				t2   = t<<1;//*2

				b = (newScreen->pBuffer[tbpp  ]) >> 3 ;		
				g = (newScreen->pBuffer[tbpp+1]) >> 2;
				r = (newScreen->pBuffer[tbpp+2]) >> 3;

				pEnc->dScreenResult.pBuffer[t2+1] = (r<<3) | ((g >> 3)&0x07);
				pEnc->dScreenResult.pBuffer[t2]   = (b)    | ((g << 5)&0xE0);

				//kocka = sor/16*pEnc->nScreenWidthP16+osz/16;
				/*kocka = (sor>>4)*pEnc->nScreenWidthP16+(osz>>4);
				if (pEnc->pScreenChanges[kocka] != 1)
					if (memcmp(  &newScreen->pBuffer[tbpp]
				           , &oldScreen->pBuffer[tbpp], 3)
							   !=0)
								pEnc->pScreenChanges[kocka] = 1;*/
				if ((osz & 0xF)==0)
				{//compare one complete line of 16 pixels
					plane = (sor>>4)*pEnc->nScreenWidthP16+(osz>>4);
					if (pEnc->pScreenChanges[plane] != 1)
					if (memcmp(  &newScreen->pBuffer[tbpp]
				           , &oldScreen->pBuffer[tbpp], 4*16)
							   !=0)
								pEnc->pScreenChanges[plane] = 1;
				}
			}
	}
}
