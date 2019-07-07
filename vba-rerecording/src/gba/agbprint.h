#ifndef VBA_AGBPRINT_H
#define VBA_AGBPRINT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern void agbPrintEnable(bool);
extern bool agbPrintIsEnabled();
extern void agbPrintReset();
extern bool agbPrintWrite(u32, u16);
extern void agbPrintFlush();

#endif // VBA_AGBPRINT_H
