// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// some build target defines:

#ifndef _WIN32_WINDOWS
///#	define _WIN32_WINDOWS 0x0410 // Windows 98 +
///#   define _WIN32_WINDOWS 0x0500 // Windows NT +
#	define _WIN32_WINDOWS 0x0501 // Windows XP +
#endif

#ifndef _WIN32_WINNT
///#	define _WIN32_WINNT 0x0410 // Windows 98 +
///#   define _WIN32_WINNT 0x0500 // Windows NT +
#	define _WIN32_WINNT 0x0501 // Windows XP +
#endif

#ifndef WINVER
///#	define WINVER 0x0410 // Windows 98 +
///#   define WINVER 0x0500 // Windows NT +
#	define WINVER 0x0501 // Windows XP +
#endif

#if !defined(AFX_STDAFX_H__A7126ECB_A234_4116_A7D0_BE50547E87F8__INCLUDED_)
#define AFX_STDAFX_H__A7126ECB_A234_4116_A7D0_BE50547E87F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Insert your headers here
//#define WIN32_LEAN_AND_MEAN           // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>

#include "../Port.h"

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A7126ECB_A234_4116_A7D0_BE50547E87F8__INCLUDED_)
