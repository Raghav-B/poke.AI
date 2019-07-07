#ifndef VBA_VERSION_H
#define VBA_VERSION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRINGIZE_VALUE(X) _Py_STRINGIZE2(X)
#define _Py_STRINGIZE(X) _Py_STRINGIZE1((X))
#define _Py_STRINGIZE1(X) _Py_STRINGIZE2 ## X
#define _Py_STRINGIZE2(X) #X
//re: http://72.14.203.104/search?q=cache:HG-okth5NGkJ:mail.python.org/pipermail/python-checkins/2002-November/030704.html+_msc_ver+compiler+version+string&hl=en&gl=us&ct=clnk&cd=5

#if defined(_MSC_VER)
#	define VBA_COMPILER ""
#	define VBA_COMPILER_DETAIL " msvc " _Py_STRINGIZE(_MSC_VER)
#else
// TODO: make for others compilers
#	define VBA_COMPILER ""
#	define VBA_COMPILER_DETAIL ""
#endif

#define VBA_NAME "VBA-RR"

#ifdef USE_GBA_CORE_V7
#	define VBA_RR_MAJOR_VERSION_NO 23
#	define VBA_RR_MINOR_VERSION_NO 6
#else
#	define VBA_RR_MAJOR_VERSION_NO 24
#	define VBA_RR_MINOR_VERSION_NO 0
#endif

#if !defined(_DEBUG) && (defined(WIN32) || defined(RC_INVOKED))
#	include "../win32/userconfig/svnrev.h"
#endif
#
#ifndef SVN_REV
#	define SVN_REV 0
#endif

#ifndef SVN_REV_STR
#	if SVN_REV > 0
#		define SVN_REV_STR STRINGIZE_VALUE(SVN_REV)
#	else
#		define define SVN_REV_STR ""
#	endif
#endif

#ifdef _DEBUG
#	define VBA_SUBVERSION_STRING " DEBUG"
#	define VBA_BUILDTYPE_STRING  "Debug"
#elif defined(PUBLIC_RELEASE)
#	define VBA_SUBVERSION_STRING ""
#	define VBA_BUILDTYPE_STRING  "Release"
#else // interim
#	define VBA_SUBVERSION_STRING " svn" SVN_REV_STR
#	define VBA_BUILDTYPE_STRING  "Interim"
#endif

#define VBA_FEATURE_STRING ""

#if VBA_RR_MINOR_VERSION_NO > 0
#	define VBA_RR_VERSION_NO VBA_RR_MAJOR_VERSION_NO ## . ## VBA_RR_MINOR_VERSION_NO
#else
#	define VBA_RR_VERSION_NO VBA_RR_MAJOR_VERSION_NO
#endif

#define VBA_VERSION_STRING "v" STRINGIZE_VALUE(VBA_RR_VERSION_NO) VBA_SUBVERSION_STRING VBA_FEATURE_STRING VBA_COMPILER
#define VBA_NAME_AND_VERSION VBA_NAME " " VBA_VERSION_STRING
#define VBA_RR_SITE "http://code.google.com/p/vba-rerecording/"

#endif // !VBA_VERSION_H
