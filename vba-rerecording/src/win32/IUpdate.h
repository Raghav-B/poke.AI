#ifndef VBA_WIN32_IUPDATE_H
#define VBA_WIN32_IUPDATE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class IUpdateListener
{
public:
	virtual void update() = 0;
};

#endif
