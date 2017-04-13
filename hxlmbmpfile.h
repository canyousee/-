#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "math.h"
#include "windows.h"

#ifndef HXLBMPFILEH
#define HXLBMPFILEH
class HXLBMPFILE
{
	BYTE *Imagedata;
public:
	int imagew, imageh; //宽和高上各有多少个像素点
	int iYRGBnum;//1：灰度，3：彩色
	RGBQUAD palette[256];

	BYTE *pDataAt(int h, int Y0R0G1B2 = 0);
	BOOL AllocateMem();
	BOOL LoadBMPFILE(char *fname);
	BOOL SaveBMPFILE(char *fname);

	HXLBMPFILE();
	~HXLBMPFILE();
};
#endif
