
#ifndef VBA_FILTERS_H
#define VBA_FILTERS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern void Pixelate2x16(u8*, u32, u8*, u8*, u32, int, int);
extern void Pixelate2x32(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Pixelate3x16)(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Pixelate3x32)(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Pixelate4x16)(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Pixelate4x32)(u8*, u32, u8*, u8*, u32, int, int);
extern void MotionBlur(u8*, u32, u8*, u8*, u32, int, int);
extern void MotionBlur32(u8*, u32, u8*, u8*, u32, int, int);
extern void _2xSaI(u8*, u32, u8*, u8*, u32, int, int);
extern void _2xSaI32(u8*, u32, u8*, u8*, u32, int, int);
extern void Super2xSaI(u8*, u32, u8*, u8*, u32, int, int);
extern void Super2xSaI32(u8*, u32, u8*, u8*, u32, int, int);
extern void SuperEagle(u8*, u32, u8*, u8*, u32, int, int);
extern void SuperEagle32(u8*, u32, u8*, u8*, u32, int, int);
extern void AdMame2x(u8*, u32, u8*, u8*, u32, int, int);
extern void AdMame2x32(u8*, u32, u8*, u8*, u32, int, int);
extern void Simple2x16(u8*, u32, u8*, u8*, u32, int, int);
extern void Simple2x32(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Simple3x16)(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Simple3x32)(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Simple4x16)(u8*, u32, u8*, u8*, u32, int, int);
extern void (*Simple4x32)(u8*, u32, u8*, u8*, u32, int, int);
extern void Bilinear(u8*, u32, u8*, u8*, u32, int, int);
extern void Bilinear32(u8*, u32, u8*, u8*, u32, int, int);
extern void BilinearPlus(u8*, u32, u8*, u8*, u32, int, int);
extern void BilinearPlus32(u8*, u32, u8*, u8*, u32, int, int);
extern void Scanlines(u8*, u32, u8*, u8*, u32, int, int);
extern void Scanlines32(u8*, u32, u8*, u8*, u32, int, int);
extern void ScanlinesTV(u8*, u32, u8*, u8*, u32, int, int);
extern void ScanlinesTV32(u8*, u32, u8*, u8*, u32, int, int);
extern void hq2x(u8*, u32, u8*, u8*, u32, int, int);
extern void hq2x32(u8*, u32, u8*, u8*, u32, int, int);
extern void hq2xS(u8*, u32, u8*, u8*, u32, int, int);
extern void hq2xS32(u8*, u32, u8*, u8*, u32, int, int);
extern void lq2x(u8*, u32, u8*, u8*, u32, int, int);
extern void lq2x32(u8*, u32, u8*, u8*, u32, int, int);
extern void hq3x(u8*, u32, u8*, u8*, u32, int, int);
extern void hq3x32(u8*, u32, u8*, u8*, u32, int, int);
extern void hq3xS(u8*, u32, u8*, u8*, u32, int, int);
extern void hq3xS32(u8*, u32, u8*, u8*, u32, int, int);

extern void SmartIB(u8*, u32, int, int);
extern void SmartIB32(u8*, u32, int, int);
extern void MotionBlurIB(u8*, u32, int, int);
extern void InterlaceIB(u8*, u32, int, int);
extern void MotionBlurIB32(u8*, u32, int, int);

extern void InterframeCleanup();

#endif // VBA_FILTERS_H
