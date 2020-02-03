

#include <stdio.h>
#include <string.h>

#include "mbl_encoder_32_16.h"
#include "mbl_encoder.h"

#define MBL_SQUARE_SIZE 512
#define MBL_RAW_FRAME_HEADER 0x0002
#define MBL_RAW_FRAME_ENDER  0xFFFE

typedef struct _MBL_COORD MBL_COORD;
struct _MBL_COORD
{
	unsigned char nX;
	unsigned char nY;
};

typedef struct _MBL_HEADER MBL_HEADER;
struct _MBL_HEADER
{//512: T(2Byte) N(2byte) 252*X(1byte)/Y(1byte) W(4Byte)
	unsigned short	nFrameType;//T(2Byte)
	unsigned short	nSquareCount;//N(2byte)
	MBL_COORD		dCoords[252];
	unsigned char	cReserved[4];
};

int _mbl_encode_raw_32bit_2_16bit(MBL_ENCODER* pEnc, unsigned char* pData)
{
	//mbl_resizebuffer 

	unsigned int square_added = 0;
	unsigned int pos_buffer   = 0;
	MBL_HEADER* pLastHeader   = NULL;//(MBLC_HEADER*)pEnc->dBuffer4Encoding;

	unsigned int sx,sy;//square xy
	unsigned int px,py;//pixel  xy

	//for (int square=0;square<252;square++)
	for (unsigned int square=0;square<pEnc->nScreenSquares;square++)
	{
		if ((square_added % 252) == 0)
		{//add header
			pLastHeader   = (MBL_HEADER*)(pEnc->dBuffer4Encoding.pBuffer+pos_buffer);
			memset(pLastHeader, 0, MBL_SQUARE_SIZE);

			pLastHeader->nFrameType    = MBL_RAW_FRAME_HEADER; 
			pLastHeader->nSquareCount  = 0;

			pos_buffer += MBL_SQUARE_SIZE;
			square_added++;
		}

		if (pEnc->pScreenChanges[square])
		{
			sx = square % pEnc->nScreenWidthP16;
			sy = square / pEnc->nScreenWidthP16;

			px = sx << 4;
			py = sy << 4;

			pLastHeader->dCoords[pLastHeader->nSquareCount].nX = sx & 0xFF;
			pLastHeader->dCoords[pLastHeader->nSquareCount].nY = sy & 0xFF;

			for (int i=0;i<16;i++)
				memcpy(	  pEnc->dBuffer4Encoding.pBuffer+pos_buffer +i*16*2
						, pEnc->dScreenResult.pBuffer+(((py*pEnc->nScreenWidth)+px+(i*pEnc->nScreenWidth))*2) // square*MBL_SQUARE_SIZE +i*16*2
						, 16*2);

//			memset(pEnc->dBuffer4Encoding.pBuffer+pos_buffer, 0xFF, 512);
			
			pLastHeader->nSquareCount++;
			square_added ++;
			pos_buffer += MBL_SQUARE_SIZE;
		}
	}

	pLastHeader   = (MBL_HEADER*)(pEnc->dBuffer4Encoding.pBuffer+pos_buffer);
	memset(pLastHeader, 0, MBL_SQUARE_SIZE);
	pLastHeader->nFrameType = MBL_RAW_FRAME_ENDER; 
	pos_buffer += MBL_SQUARE_SIZE;

	pEnc->dBuffer4Encoding.nBufferSizeUsed = pos_buffer;

	/*FILE* f = fopen("test.txt", "wt");
	if (f)
	{
		for (int i=0;i<pos_buffer;i++)
		{
			if ((i%512)==0)
				fprintf(f, "\n\n");
			
			fprintf(f, "%02x ", pEnc->dBuffer4Encoding.pBuffer[i]);
		}
		fclose(f);
	}*/

	if (square_added <= 1) 
		 return 0; //no changes
	else return square_added-1;
}

