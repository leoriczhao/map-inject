#pragma once

#if defined(_MSC_VER)
#elif defined(__GCC__) || defined(__GNUC__)
#	include <slk/port/port_gcc.h>
#else
#	error unknown compiler
#endif

// We build SlkLib inline (not as a separate DLL)
#define SLKLIB_INLINE
#define SLKLIB_API

#pragma warning(disable:4251)
