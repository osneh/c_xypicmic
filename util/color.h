#ifndef H_COLOR_151113
#define H_COLOR_151113
/**
 * File: color.h
 *
 * About: Description
 * This file provides coloring utility for C programs running in a terminal.
 * It is aimed to be cross-platform (at least Windows and Linux).
 * 
 *
 * About: Based on rtutil.h (Tapio Vierros)
 *
 */



/// Define: COLOR_USE_ANSI
/// Define this to use ANSI escape sequences also on Windows
/// (defaults to using WinAPI instead).
#if 0
#define COLOR_USE_ANSI
#endif


#define COLOR_HAS_COLOR_FOR_THE_TERM

#if defined(_WIN32) && !defined(COLOR_USE_ANSI)
	#include <windows.h>  // for WinAPI
	#define _NO_OLDNAMES  // for MinGW compatibility
#endif // _WIN32

#include <stdio.h> 

/**
 * Enums: Color codes
 *
 * BLACK - Black
 * BLUE - Blue
 * GREEN - Green
 * CYAN - Cyan
 * RED - Red
 * MAGENTA - Magenta / purple
 * BROWN - Brown / dark yellow
 * GREY - Grey / dark white
 * DARKGREY - Dark grey / light black
 * LIGHTBLUE - Light blue
 * LIGHTGREEN - Light green
 * LIGHTCYAN - Light cyan
 * LIGHTRED - Light red
 * LIGHTMAGENTA - Light magenta / light purple
 * YELLOW - Yellow (bright)
 * WHITE - White (bright)
 */
enum {
	BLACK,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	GREY,
	DARKGREY,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE,
    DEFAULT
};

/// Function: setColor
/// Change color specified by number (Windows / QBasic colors).
///
/// See <Color Codes>
void setColor(int c);

/// Function: getANSIColor
/// Return ANSI color escape sequence for specified number 0-15.
///
/// See <Color Codes>
const char* getANSIColor(const int c);

#endif // H_COLOR_151113
