#ifndef __MBL_ENCODER_H__ 
#define __MBL_ENCODER_H__ 

#include <stdlib.h>

#define MBL_ENCODE_MASK			0x7FFF
#define MBL_ENCODE_CHANGEONLY	0x8000
#define MBL_ENCODE_RAW_16BIT	0

typedef struct _MBL_BUFFER MBL_BUFFER;

struct _MBL_BUFFER
{
	unsigned char*  pBuffer;//two buffers to check the changes
	unsigned int    nBufferSize;//real size
	unsigned int    nBufferSizeUsed;//used size (must be <= than nBufferSize)
};

typedef struct _MBL_ENCODER MBL_ENCODER;

struct _MBL_ENCODER
{
	int				nEncodeType;
	
	MBL_BUFFER		dBuffer4Encoding;

	unsigned int	nScreenWidth;
	unsigned int	nScreenHeight;
	unsigned int	nScreenWidthP16;//width/16
	unsigned int	nScreenHeightP16;//height/16
	unsigned int	nScreenSquares;//calculated always from nScreenWidth and nScreenHeight
	unsigned int	nScreenSize32bit;
	
	unsigned int	nPacketSize16bit;

	MBL_BUFFER      dScreens[2];//two buffers to check the changes
	unsigned char*	pScreenChanges;
	int				nScreenActive;//which one should be used on the next write

	MBL_BUFFER      dScreenResult;//the result of the screen color shift
};

MBL_ENCODER* mbl_createEncoder(int nEncType, unsigned int nScreenWidth, unsigned int nScreenHeight);
void mbl_deleteEncoder(MBL_ENCODER* pEnc);
int mbl_encode_from_32bit(MBL_ENCODER* pEnc, unsigned char* pData);

#endif
