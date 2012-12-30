// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef JPEG_H
#define JPEG_H

#include "OMXComponent.h"
#include "ILogger.h"
#include "VG/openvg.h"
#include "VG/vgu.h"


class OMXJPEG
{
public:
	OMXJPEG(ILogger *pLogger);

	VGImage CreateImageFromBuf(unsigned char *buf, unsigned int bufLength, unsigned int outputWidth, unsigned int outputHeight);

private:


	void OnDecoderOutputChanged(unsigned int * outputWidth, unsigned int * outputHeight);

	void OnDecoderOutputChangedAgain();

	///////

	IOMXComponent *m_pCompDecode, *m_pCompResize;
	ILogger *m_pLogger;
	int m_iInPortDecode, m_iOutPortDecode;
	int m_iInPortResize, m_iOutPortResize;
	void *m_pBufOutput;
	OMX_BUFFERHEADERTYPE *m_pHeaderOutput;
};

#endif // JPEG_H
