#pragma once

#include "mbl_encoder.h"

/** Resize a buffer if needed
@return new size (0 if unsuccessfull)
*/
unsigned int mbl_resizebuffer(MBL_BUFFER* pBuffer, unsigned int nNewSize);

