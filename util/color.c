#include "color.h"
/**
 * Consts: ANSI color strings
 *
 * ANSI_CLS - Clears screen
 * ANSI_BLACK - Black
 * ANSI_RED - Red
 * ANSI_GREEN - Green
 * ANSI_BROWN - Brown / dark yellow
 * ANSI_BLUE - Blue
 * ANSI_MAGENTA - Magenta / purple
 * ANSI_CYAN - Cyan
 * ANSI_GREY - Grey / dark white
 * ANSI_DARKGREY - Dark grey / light black
 * ANSI_LIGHTRED - Light red
 * ANSI_LIGHTGREEN - Light green
 * ANSI_YELLOW - Yellow (bright)
 * ANSI_LIGHTBLUE - Light blue
 * ANSI_LIGHTMAGENTA - Light magenta / light purple
 * ANSI_LIGHTCYAN - Light cyan
 * ANSI_WHITE - White (bright)
 */
//static const char* ANSI_CLS = "\033[2J";
static const char* ANSI_BLACK = "\033[22;30m";
static const char* ANSI_RED = "\033[22;31m";
static const char* ANSI_GREEN = "\033[22;32m";
static const char* ANSI_BROWN = "\033[22;33m";
static const char* ANSI_BLUE = "\033[22;34m";
static const char* ANSI_MAGENTA = "\033[22;35m";
static const char* ANSI_CYAN = "\033[22;36m";
static const char* ANSI_GREY = "\033[22;37m";
static const char* ANSI_DARKGREY = "\033[01;30m";
static const char* ANSI_LIGHTRED = "\033[01;31m";
static const char* ANSI_LIGHTGREEN = "\033[01;32m";
static const char* ANSI_YELLOW = "\033[01;33m";
static const char* ANSI_LIGHTBLUE = "\033[01;34m";
static const char* ANSI_LIGHTMAGENTA = "\033[01;35m";
static const char* ANSI_LIGHTCYAN = "\033[01;36m";
static const char* ANSI_WHITE = "\033[01;37m";
static const char* ANSI_DEFAULT = "\033[0m";


/// Function: getANSIColor
/// Return ANSI color escape sequence for specified number 0-15.
///
/// See <Color Codes>
const char* getANSIColor(const int c) 
{
	switch (c) {
		case 0 : return ANSI_BLACK;
		case 1 : return ANSI_BLUE; // non-ANSI
		case 2 : return ANSI_GREEN;
		case 3 : return ANSI_CYAN; // non-ANSI
		case 4 : return ANSI_RED; // non-ANSI
		case 5 : return ANSI_MAGENTA;
		case 6 : return ANSI_BROWN;
		case 7 : return ANSI_GREY;
		case 8 : return ANSI_DARKGREY;
		case 9 : return ANSI_LIGHTBLUE; // non-ANSI
		case 10: return ANSI_LIGHTGREEN;
		case 11: return ANSI_LIGHTCYAN; // non-ANSI;
		case 12: return ANSI_LIGHTRED; // non-ANSI;
		case 13: return ANSI_LIGHTMAGENTA;
		case 14: return ANSI_YELLOW; // non-ANSI
		case 15: return ANSI_WHITE;
        case 16: return ANSI_DEFAULT;
		default: return "";
	}
}

/// Function: setColor
/// Change color specified by number (Windows / QBasic colors).
///
/// See <Color Codes>
void setColor(int c) 
{
#if defined(_WIN32) && !defined(COLOR_USE_ANSI)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    c = (c == 16) ? 15 : c; // back to white if ANSI_DEFAULT
	SetConsoleTextAttribute(hConsole, (WORD)c);
#else
	fputs(getANSIColor(c), stdout);
#endif
}
