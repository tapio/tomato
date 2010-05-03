#pragma once

#include <string>
#include <GL/gl.h>

#include "util.hh"

/// Warning: These functions are not UTF-8 capable so other than ASCII
/// characters are drawn incorrectly save for characters Ä, ä, Ö and ö which 
/// are emulated.

enum FontAlign {
	ALIGN_LEFT = 0,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

/// Draw text starting at the given position
void drawText(GLuint texHandle, Color color, const float x, const float y, const std::string& text, FontAlign align = ALIGN_CENTER);

/// Build displaylists for ISO-8859-1/Windows-1252 font sheet.
void buildFonts();

// TODO: Detele lists
