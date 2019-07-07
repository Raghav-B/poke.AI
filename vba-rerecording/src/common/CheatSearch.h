#ifndef VBA_CHEATSEARCH_H
#define VBA_CHEATSEARCH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Port.h"

struct CheatSearchBlock
{
	u8 *data;
	int size;
	u32 offset;
	u8 *saved;
	u8 *bits;
};

struct CheatSearchData
{
	int count;
	CheatSearchBlock *blocks;
};

enum
{
	SEARCH_EQ,
	SEARCH_NE,
	SEARCH_LT,
	SEARCH_LE,
	SEARCH_GT,
	SEARCH_GE
};

enum
{
	BITS_8,
	BITS_16,
	BITS_32
};

#define SET_BIT(bits, off) \
    (bits)[(off) >> 3] |= (1 << ((off) & 7))

#define CLEAR_BIT(bits, off) \
    (bits)[(off) >> 3] &= ~(1 << ((off) & 7))

#define IS_BIT_SET(bits, off) \
    (bits)[(off) >> 3] & (1 << ((off) & 7))

extern CheatSearchData cheatSearchData;
extern void cheatSearchSetSavedAndBits(CheatSearchBlock *block);
extern void cheatSearchZeroBlock(CheatSearchBlock *block);
extern void cheatSearchCleanup(CheatSearchData *cs);
extern void cheatSearchStart(const CheatSearchData *cs);
extern void cheatSearch(const CheatSearchData *cs, int compare, int size,
                        bool isSigned);
extern void cheatSearchValue(const CheatSearchData *cs, int compare, int size,
                             bool isSigned, u32 value);
extern int cheatSearchGetCount(const CheatSearchData *cs, int size);
extern void cheatSearchUpdateValues(const CheatSearchData *cs);
extern s32 cheatSearchSignedRead(u8 *data, int off, int size);
extern u32 cheatSearchRead(u8 *data, int off, int size);

#endif // VBA_CHEATSEARCH_H
