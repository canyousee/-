#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "math.h"
#include "windows.h"
#include"HXMBMPFILE.h"


HXLBMPFILE::HXLBMPFILE()
{
	Imagedata = NULL;
	for (int i = 0; i< 256; i++)
	{
		palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = i;
		palette[i].rgbReserved = 0;
	}
	iYRGBnum = 1;//默认是灰度图
	imagew = imageh = 0;
}

HXLBMPFILE::~HXLBMPFILE()
{
	if (Imagedata) delete Imagedata;
}


BYTE *HXLBMPFILE::pDataAt(int h, int Y0R0G1B2) //h表第几行
{
	if (iYRGBnum <= Y0R0G1B2) return NULL;
	int w = imagew *h + Y0R0G1B2 * imagew * imageh;
	return Imagedata + w; //定位在内存中的位置
}

BOOL HXLBMPFILE::AllocateMem()
{
	int w = imagew * imageh * iYRGBnum;

	if (Imagedata)
	{
		delete Imagedata;
		Imagedata = NULL;
	}
	Imagedata = new BYTE[w];
	if (Imagedata) memset(Imagedata, 0, w);  //把申请的这部分空间填充为0
	return (Imagedata != NULL);
}

BOOL HXLBMPFILE::LoadBMPFILE(char *cFilename)
{
	FILE *f;
	if (strlen(cFilename)<1) return FALSE;
	/*errno_t  err;
	err = fopen_s(&f,cFilename, "rb");*/
	f = fopen(cFilename, "rb");
	if (f == NULL) return FALSE;  //打开文件

	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER ih;
	fread(&fh, sizeof(BITMAPFILEHEADER), 1, f);
	if (fh.bfType != 0x4d42) { fclose(f); return FALSE; }//"BM"

	fread(&ih, sizeof(BITMAPINFOHEADER), 1, f);
	if ((ih.biBitCount != 8) && (ih.biBitCount != 24))   //一个像素点是多少位
	{
		fclose(f);
		return FALSE;
	}

	iYRGBnum = ih.biBitCount / 8;
	imagew = ih.biWidth;
	imageh = ih.biHeight;
	if (!AllocateMem()) { fclose(f); return FALSE; }
	if (iYRGBnum == 1) fread(palette, sizeof(RGBQUAD), 256, f); //从f中读256块RGBQUAD
	fseek(f, fh.bfOffBits, SEEK_SET);

	int w4b = (imagew * iYRGBnum + 3) / 4 * 4;   //每行有w4b个字节
	int i, j;
	BYTE *ptr;

	if (iYRGBnum == 1)
	{
		w4b -= imagew; //填补的空字节
		for (i = imageh - 1; i >= 0; i--)
		{
			fread(pDataAt(i), imagew, 1, f);
			if (w4b > 0) fseek(f, w4b, SEEK_CUR);
		}
	}

	if (iYRGBnum == 3)
	{
		ptr = new BYTE[w4b];
		for (i = imageh - 1; i >= 0; i--)
		{
			fread(ptr, w4b, 1, f);  //读入一行
			for (j = 0; j < imagew; j++)
			{
				*(pDataAt(i, 2) + j) = *(ptr + j * 3 + 0);  //R<->B
				*(pDataAt(i, 1) + j) = *(ptr + j * 3 + 1);   //G<->G
				*(pDataAt(i, 0) + j) = *(ptr + j * 3 + 2);  //B<->R
			}
		}
		delete ptr;
	}
	fclose(f);
	return TRUE;
}


BOOL HXLBMPFILE::SaveBMPFILE(char *cFilename)
{
	if (!Imagedata) return FALSE;
	FILE *f;
	if (strlen(cFilename)<1) return FALSE;
	/*errno_t  err;
	err = fopen_s(&f, cFilename, "w+b");*/
	f = fopen(cFilename, "w+b");
	if (f == NULL) return FALSE;

	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER ih;
	memset(&ih, 0, sizeof(BITMAPINFOHEADER));
	fh.bfType = 0x4d42;
	fh.bfReserved1 = fh.bfReserved2 = 0;
	fh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+
		((iYRGBnum == 1) ? 256 * sizeof(RGBQUAD) : 0);

	ih.biSize = 40;
	ih.biPlanes = 1;
	ih.biWidth = imagew;
	ih.biHeight = imageh;
	ih.biBitCount = 8 * iYRGBnum;
	int w4b = (imagew*iYRGBnum + 3) / 4 * 4;
	ih.biSizeImage = ih.biHeight *w4b;
	fh.bfSize = fh.bfOffBits + ih.biSizeImage;
	fwrite(&fh, sizeof(BITMAPFILEHEADER), 1, f);
	fwrite(&ih, sizeof(BITMAPINFOHEADER), 1, f);
	if (iYRGBnum == 1) fwrite(palette, sizeof(RGBQUAD), 256, f);

	BYTE* ptr;
	int i, j;

	if (iYRGBnum == 1)
	{
		ptr = new BYTE[10];
		memset(ptr, 0, 10); //最多只能有三个啊？？
		w4b -= ih.biWidth;
		for (i = ih.biHeight - 1; i >= 0; i--)
		{
			fwrite(pDataAt(i), ih.biWidth, 1, f);
			if (w4b>0) fwrite(ptr, w4b, 1, f);
		}
		delete ptr;
	}

	if (iYRGBnum == 3)
	{
		ptr = new BYTE[w4b];
		memset(ptr, 0, w4b);
		for (i = ih.biHeight - 1; i >= 0; i--)
		{
			for (j = 0; j < ih.biWidth; j++)
			{
				*(ptr + j * 3 + 0) = *(pDataAt(i, 2) + j);  //硬盘=内存
				*(ptr + j * 3 + 1) = *(pDataAt(i, 1) + j);
				*(ptr + j * 3 + 2) = *(pDataAt(i, 0) + j);
			}
			fwrite(ptr, w4b, 1, f);
		}
		delete ptr;
	}
	fclose(f);
	return TRUE;
}
