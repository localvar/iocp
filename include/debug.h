#pragma once


// The default buffer size used to store debug message
#ifndef __DEBUG_BUFFER_SIZE__
#define __DEBUG_BUFFER_SIZE__ 255
#endif


// Write some message to the debugger, the next two are the real versions
inline void __cdecl Trace_( LPCSTR pszFmt, ... )
{
	va_list args;
	va_start( args, pszFmt );
	char sz[__DEBUG_BUFFER_SIZE__ + 1];
	_vsnprintf( sz, __DEBUG_BUFFER_SIZE__, pszFmt, args );
	sz[__DEBUG_BUFFER_SIZE__] = 0;
	OutputDebugStringA( sz );
	va_end( args );
}

inline void __cdecl Trace_( LPCWSTR pszFmt, ... )
{
	va_list args;
	va_start( args, pszFmt );
	WCHAR sz[__DEBUG_BUFFER_SIZE__ + 1];
	_vsnwprintf( sz, __DEBUG_BUFFER_SIZE__, pszFmt, args );
	sz[__DEBUG_BUFFER_SIZE__] = 0;
	OutputDebugStringW( sz );
	va_end( args );
}

// These two are the fake versions
inline void __cdecl Fake_Trace_( LPCSTR pszFmt, ... )
{
	pszFmt;
	// do nothing
}
inline void __cdecl Fake_Trace_( LPCWSTR pszFmt, ... )
{
	pszFmt;
	// do nothing
}


#ifndef ASSERT
#define ASSERT(x) _ASSERTE(x)
#endif // ASSERT

// The Verify Macro
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(x) _ASSERTE(x)
#else
#define VERIFY(x) (x)
#endif // _DEBUG
#endif // VERIFY



// The Trace Macro
#ifndef TRACE
#ifdef _DEBUG
#define TRACE Trace_
#else
#define TRACE Fake_Trace_
#endif // _DEBUG
#endif // TRACE