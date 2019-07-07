#include "../Port.h"

extern u32 RGB_LOW_BITS_MASK;

void Pixelate2x16(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
                  u8 *dstPtr, u32 dstPitch, int width, int height)
{
	u8 *nextLine, *finish;
	u32 colorMask = ~(RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 16));
	colorMask = (colorMask >> 2) & (colorMask >> 1);

	nextLine = dstPtr + dstPitch;

	do
	{
		u32 *bP = (u32 *) srcPtr;
		u32 *xP = (u32 *) deltaPtr;
		u32 *dP = (u32 *) dstPtr;
		u32 *nL = (u32 *) nextLine;
		u32  currentPixel;
		u32  nextPixel;
		u32  currentDelta;
		u32  nextDelta;

		finish    = (u8 *) bP + ((width+2) << 1);
		nextPixel = *bP++;
		nextDelta = *xP++;

		do
		{
			currentPixel = nextPixel;
			currentDelta = nextDelta;
			nextPixel    = *bP++;
			nextDelta    = *xP++;

			if ((nextPixel != nextDelta) || (currentPixel != currentDelta))
			{
				u32 colorA, colorB, product;

				*(xP - 2) = currentPixel;
#ifdef WORDS_BIGENDIAN
				colorA = currentPixel >> 16;
				colorB = currentPixel & 0xffff;
#else
				colorA = currentPixel & 0xffff;
				colorB = currentPixel >> 16;
#endif
				product = (colorA >> 2) & colorMask;

#ifdef WORDS_BIGENDIAN
				*(nL) = (product << 16) | (product);
				*(dP) = (colorA << 16) | product;
#else
				*(nL) = product | (product << 16);
				*(dP) = colorA | (product << 16);
#endif

#ifdef WORDS_BIGENDIAN
				colorA = nextPixel >> 16;
#else
				colorA = nextPixel & 0xffff;
#endif
				product = (colorB >> 2) & colorMask;
#ifdef WORDS_BIGENDIAN
				*(nL + 1) = (product << 16) | (product);
				*(dP + 1) = (colorB << 16) | (product);
#else
				*(nL + 1) = (product) | (product << 16);
				*(dP + 1) = (colorB) | (product << 16);
#endif
			}

			dP += 2;
			nL += 2;
		}
		while ((u8 *) bP < finish);

		deltaPtr += srcPitch;
		srcPtr   += srcPitch;
		dstPtr   += dstPitch << 1;
		nextLine += dstPitch << 1;
	}
	while (--height);
}

void Pixelate2x32(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                  u8 *dstPtr, u32 dstPitch, int width, int height)
{
	u8 *nextLine, *finish;
	u32 colorMask = ((u32)~RGB_LOW_BITS_MASK >> 2) & ((u32)~RGB_LOW_BITS_MASK >> 1);

	nextLine = dstPtr + dstPitch;

	do
	{
		u32 *bP = (u32 *) srcPtr;
		//    u32 *xP = (u32 *) deltaPtr;
		u32 *dP = (u32 *) dstPtr;
		u32 *nL = (u32 *) nextLine;
		u32  currentPixel;
		u32  nextPixel;

		finish    = (u8 *) bP + ((width+1) << 2);
		nextPixel = *bP++;

		do
		{
			u32 product;

			currentPixel = nextPixel;
			nextPixel    = *bP++;
			product = (currentPixel >> 2) & colorMask;
			*(nL)   = product;
			*(nL+1) = product;
			*(dP)   = currentPixel;
			*(dP+1) = product;

			currentPixel = nextPixel;
			nextPixel = *bP++;
			product   = (currentPixel >> 2) & colorMask;
			*(nL + 2) = product;
			*(nL + 3) = product;
			*(dP + 2) = currentPixel;
			*(dP + 3) = product;

			dP += 4;
			nL += 4;
		}
		while ((u8 *) bP < finish);

		srcPtr   += srcPitch;
		dstPtr   += dstPitch << 1;
		nextLine += dstPitch << 1;
	}
	while (--height);
}

// generic Pixelate Nx magnification filter
template <int magnification, typename ColorType>
void PixelateNx(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
	ColorType colorMask = ((ColorType)~RGB_LOW_BITS_MASK >> 2) & ((ColorType)~RGB_LOW_BITS_MASK >> 1);

	srcPitch      = srcPitch / sizeof(ColorType) - width;
	u32 dstNextP  = dstPitch / sizeof(ColorType);
	u32 dstNextL  = (dstNextP - width) * magnification; // skip to the next magnificated 'line'
	dstNextP     -= magnification;

	u32 offset    = (dstPitch + sizeof(ColorType)) * magnification - dstPitch;

	ColorType *src   = (ColorType *)srcPtr;
	ColorType *dst   = (ColorType *)dstPtr;

	do // per src line
	{
		u8 *finishP = (u8 *)dst + offset;
		for (int x = 0; x < width; ++x) // per pixel in line
		{
			ColorType col    = *src;
			ColorType *dst2  = dst;
			u8 *finishM = (u8 *)(dst + magnification);

			ColorType product = (col >> 2) & colorMask;
			do
			{
				*dst2 = product;
			} while ((u8 *)++dst2 < finishM);
			dst2    += dstNextP;
			finishM += dstPitch;
			do // dst magnificated pixel
			{
				*dst2++ = product;
				do
				{
					*dst2 = col;
				} while ((u8 *)++dst2 < finishM);
				dst2    += dstNextP;
				finishM += dstPitch;
			} while ((u8 *)dst2 < finishP);

			++src;
			dst     += magnification;
			finishP += magnification * sizeof(ColorType);
		}
		src += srcPitch;
		dst += dstNextL;
	} while (--height);
}

typedef void (*PixelateNxFP)(u8*, u32, u8*, u8*, u32, int, int);

PixelateNxFP Pixelate3x16 = PixelateNx<3, u16>;
PixelateNxFP Pixelate3x32 = PixelateNx<3, u32>;
PixelateNxFP Pixelate4x16 = PixelateNx<4, u16>;
PixelateNxFP Pixelate4x32 = PixelateNx<4, u32>;
