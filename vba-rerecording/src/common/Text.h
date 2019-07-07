#ifndef VBA_TEXT_H
#define VBA_TEXT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern void drawText(u8 *, int, int, int, const char *, const char*cl = NULL);
extern void drawTextTransp(u8 *, int, int, int, const char *, const char*cl = NULL);

extern bool outlinedText, transparentText;
extern int  textColor, textMethod;

#endif // VBA_TEXT_H
