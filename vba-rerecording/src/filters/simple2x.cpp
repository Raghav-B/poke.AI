#include "../Port.h"

void Simple2x16(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
	u8 *nextLine, *finish;

	nextLine = dstPtr + dstPitch;

	do
	{
		u32 *bP = (u32 *) srcPtr;
		u32 *dP = (u32 *) dstPtr;
		u32 *nL = (u32 *) nextLine;
		u32	 currentPixel;

		finish		 = (u8 *) bP + ((width + 2) << 1);
		currentPixel = *bP++;

		do
		{
#ifdef WORDS_BIGENDIAN
			u32 color = currentPixel >> 16;
#else
			u32 color = currentPixel & 0xffff;
#endif

			color = color | (color << 16);

			*(dP) = color;
			*(nL) = color;

#ifdef WORDS_BIGENDIAN
			color = currentPixel & 0xffff;
#else
			color = currentPixel >> 16;
#endif
			color	  = color | (color << 16);
			*(dP + 1) = color;
			*(nL + 1) = color;

			currentPixel = *bP++;

			dP += 2;
			nL += 2;
		}
		while ((u8 *) bP < finish);

		srcPtr	 += srcPitch;
		dstPtr	 += dstPitch << 1;
		nextLine += dstPitch << 1;
	}
	while (--height);
}

void Simple2x32(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
	u8 *nextLine, *finish;

	nextLine = dstPtr + dstPitch;

	do
	{
		u32 *bP = (u32 *) srcPtr;
		u32 *dP = (u32 *) dstPtr;
		u32 *nL = (u32 *) nextLine;
		u32	 currentPixel;

		finish		 = (u8 *) bP + ((width + 1) << 2);
		currentPixel = *bP++;

		do
		{
			u32 color = currentPixel;

			*(dP)	  = color;
			*(dP + 1) = color;
			*(nL)	  = color;
			*(nL + 1) = color;

			currentPixel = *bP++;

			dP += 2;
			nL += 2;
		}
		while ((u8 *) bP < finish);

		srcPtr	 += srcPitch;
		dstPtr	 += dstPitch << 1;
		nextLine += dstPitch << 1;
	}
	while (--height);
}

#if 0
// generic Simple Nx magnification filter
template <int magnification, typename ColorType>
void SimpleNx(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
	srcPitch = srcPitch / sizeof(ColorType) - width;
	u32 dstNextP = dstPitch / sizeof(ColorType);
	u32 dstNextL = (dstNextP - width) * magnification;  // skip to the next magnificated 'line'
	dstNextP -= magnification;

	u32 offset = (dstPitch + sizeof(ColorType)) * magnification - dstPitch;

	ColorType *src = (ColorType *)srcPtr;
	ColorType *dst = (ColorType *)dstPtr;

	do // per src line
	{
		u8 *finishP = (u8 *)dst + offset;
		for (int x = 0; x < width; ++x) // per pixel in line
		{
			ColorType  col	   = *src;
			ColorType *dst2	   = dst;
			u8 *	   finishM = (u8 *)(dst + magnification);
			do // dst magnificated pixel
			{
				do
				{
					*dst2 = col;
				}
				while ((u8 *)++dst2 < finishM);
				dst2	+= dstNextP;
				finishM += dstPitch;
			}
			while ((u8 *)dst2 < finishP);

			++src;
			dst		+= magnification;
			finishP += magnification * sizeof(ColorType);
		}
		src += srcPitch;
		dst += dstNextL;
	}
	while (--height);
}

#else

// generic Simple Nx magnification filter
template <int magnification, typename ColorType>
void SimpleNx(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
	srcPitch  = srcPitch / sizeof(ColorType) - width;
	dstPitch /= sizeof(ColorType);
	u32 dstBlank = (dstPitch - width) * magnification; // skip to the next magnificated 'line'
	dstPitch -= magnification;

	ColorType *src = (ColorType *)srcPtr;
	ColorType *dst = (ColorType *)dstPtr;

	do // per src line
	{
		for (int x = 0; x < width; ++x) // per pixel in src line
		{
			ColorType  col	= *src;
			ColorType *dst2 = dst;
			for (int dy = 0; dy < magnification; ++dy) // dst magnificated pixel
			{
				for (int dx = 0; dx < magnification; ++dx)
				{
					*dst2 = col;
					++dst2;
				}
				dst2 += dstPitch;
			}

			++src;
			dst += magnification;
		}
		src += srcPitch;
		dst += dstBlank;
	}
	while (--height);
}

#endif

typedef void(*SimpleNxFP)(u8 *, u32, u8 *, u8 *, u32, int, int);

SimpleNxFP SimpleNx16[5] = { NULL, SimpleNx<1, u16>, SimpleNx<2, u16>, SimpleNx<3, u16>, SimpleNx<4, u16> };
SimpleNxFP SimpleNx32[5] = { NULL, SimpleNx<1, u32>, SimpleNx<2, u32>, SimpleNx<3, u32>, SimpleNx<4, u32> };

SimpleNxFP Simple1x16 = SimpleNx<1, u16>;
SimpleNxFP Simple1x32 = SimpleNx<1, u32>;
SimpleNxFP Simple3x16 = SimpleNx<3, u16>;
SimpleNxFP Simple3x32 = SimpleNx<3, u32>;
SimpleNxFP Simple4x16 = SimpleNx<4, u16>;
SimpleNxFP Simple4x32 = SimpleNx<4, u32>;
