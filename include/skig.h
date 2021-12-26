#pragma once

#include <stereokit.h>

#if defined _WIN32 || defined __CYGWIN__
	#if defined(SKIG_EXPORT)
		#define API __declspec(dllexport)
	#elif !defined(SKIG_NO_IMPORT)
		#define API __declspec(dllimport)
	#endif
#else
	#ifdef __GNUC__
		#define API __attribute__((__visibility__("default")))
	#else
		#define API
	#endif
#endif

#if defined __cplusplus
	#define EXTERN extern "C"
#else
	#define EXTERN extern
#endif

#define SKIG_API EXTERN API

namespace skig {

SKIG_API bool skig_init();

// Save current render state to allow restoring it lter with skig_end
SKIG_API void skig_begin();

// Render draw data returned by ImGui for the current context
SKIG_API void skig_render(sk::tex_t renderTarget);

// Restore previously saved render state
SKIG_API void skig_end();

SKIG_API void skig_destroy();

}
