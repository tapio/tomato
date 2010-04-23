#pragma once

#include <string>
#include <GL/gl.h>

/// Draw text starting at the given position
void drawText(GLuint texHandle, const float x, const float y, const std::string& text);

/// Build displaylists for ISO 8859-1/Windows-1252 font sheet.
void buildFonts();

// TODO: Detele lists
