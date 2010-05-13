// Based on GLFT_Font (http://polimath.com/blog/code/glft_font/) v 0.2.1
//  by James Turk (james.p.turk@gmail.com)
//  Based on work by Marijn Haverbeke (http://marijn.haverbeke.nl)
//
// Copyright (c) 2005-2008, James Turk
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the GLFT_Font nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.


#include "font.hh"
#include <cstring>
#include <cstdio>

// Static members
FT_Library FTLibraryContainer::m_library;
FTLibraryContainer Font::library_;

std::ostream& operator<<(std::ostream& os, const StreamFlusher&) { return os.flush(); }

void Font::open(const std::string& filename, unsigned int size)
{
	const size_t MARGIN = 3;

	// Release the font if it already exists.
	if (isValid()) release();

	/// Step 1: Open the font using FreeType.
	FT_Face face;
	if (FT_New_Face(library_.getLibrary(), filename.c_str(), 0, &face))
		throw std::runtime_error("Could not load font file.");

	// Abort if this is not a scalable font.
	if (!(face->face_flags & FT_FACE_FLAG_SCALABLE) || !(face->face_flags & FT_FACE_FLAG_HORIZONTAL))
		throw std::runtime_error("Invalid font: Error setting font size.");

	// Set the font size.
	FT_Set_Pixel_Sizes(face, size, 0);

	/// Step 2: Find maxAscent/Descent to calculate imageHeight.
	size_t imageHeight = 0;
	size_t imageWidth = 256;
	int maxDescent = 0;
	int maxAscent = 0;
	size_t lineSpace = imageWidth - MARGIN;
	size_t lines = 1;
	size_t charIndex;

	for (unsigned int ch = 0; ch < NUM_CHARS; ++ch)
	{
		// Look up the character in the font file.
		charIndex = FT_Get_Char_Index(face, ch+SPACE);
		// Render the current glyph.
		FT_Load_Glyph(face, charIndex, FT_LOAD_RENDER);

		widths_[ch] = (face->glyph->metrics.horiAdvance >> 6) + MARGIN;
		// If the line is full go to the next line.
		if (widths_[ch] > lineSpace) {
			lineSpace = imageWidth - MARGIN;
			++lines;
		}
		lineSpace -= widths_[ch];

		maxAscent = std::max(face->glyph->bitmap_top, maxAscent);
		maxDescent = std::max(face->glyph->bitmap.rows -
			face->glyph->bitmap_top, maxDescent);
	}

	height_ = maxAscent + maxDescent;   // Calculate height for text.

	// Compute how high the texture has to be.
	size_t neededHeight = (maxAscent + maxDescent + MARGIN) * lines + MARGIN;
	// Get the first power of two in which it will fit.
	imageHeight = 16;
	while (imageHeight < neededHeight) imageHeight <<= 1;

	/// Step 3: Generation of the actual texture.

	// Create and zero the memory.
	std::vector<unsigned char> image(imageHeight * imageWidth, 0);

	// These are the position at which to draw the next glyph.
	size_t x = MARGIN;
	size_t y = MARGIN + maxAscent;
	float texX1, texX2, texY1, texY2;   // Used for display list.

	listBase_ = glGenLists(NUM_CHARS);  // Generate the lists for filling.

	// Drawing loop.
	for (unsigned int ch = 0; ch < NUM_CHARS; ++ch)
	{
		size_t charIndex = FT_Get_Char_Index(face, ch+SPACE);

		// Render the glyph.
		FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		// See whether the character fits on the current line.
		if (widths_[ch] > imageWidth - x) {
			x = MARGIN;
			y += (maxAscent + maxDescent + MARGIN);
		}

		// Calculate texture coordinates of the character.
		texX1 = static_cast<float>(x) / imageWidth;
		texX2 = static_cast<float>(x+widths_[ch]) / imageWidth;
		texY1 = static_cast<float>(y - maxAscent) / imageHeight;
		texY2 = texY1 + static_cast<float>(height_) / imageHeight;

		// Generate the character's display list.
		glNewList(listBase_ + ch, GL_COMPILE);
			glBegin(GL_QUADS);
				glTexCoord2f(texX1,texY1);  glVertex2i(0,0);
				glTexCoord2f(texX2,texY1);  glVertex2i(widths_[ch],0);
				glTexCoord2f(texX2,texY2);  glVertex2i(widths_[ch],height_);
				glTexCoord2f(texX1,texY2);  glVertex2i(0,height_);
			glEnd();
			glTranslatef(widths_[ch],0,0);  // Translate forward.
		glEndList();

		// Copy image generated by FreeType to the texture.
		for (int row = 0; row < face->glyph->bitmap.rows; ++row) {
			for (int pixel = 0; pixel < face->glyph->bitmap.width; ++pixel) {
				// Set pixel at position to intensity (0-255) at the position.
				image[(x + face->glyph->bitmap_left + pixel) +
					(y - face->glyph->bitmap_top + row) * imageWidth] =
						face->glyph->bitmap.buffer[pixel +
							row * face->glyph->bitmap.pitch];
			}
		}
		x += widths_[ch];
	}

	// Generate the OpenGL texture from the byte array.
	glGenTextures(1, &texID_);
	glBindTexture(GL_TEXTURE_2D, texID_);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, imageWidth, imageHeight, 0,
					GL_ALPHA, GL_UNSIGNED_BYTE, &image[0]);

	FT_Done_Face(face); // Free the face data.
}

void Font::release()
{
	if (glIsList(listBase_)) glDeleteLists(listBase_, NUM_CHARS);
	if (glIsTexture(texID_)) glDeleteTextures(1, &texID_);

	// Clear out data.
	texID_ = 0;
	listBase_ = 0;
	widths_.clear();
	widths_.resize(NUM_CHARS);
	height_ = 0;
}

void Font::drawText(float x, float y, const char *str, ...) const
{
	if (!isValid()) throw std::logic_error("Invalid Font::drawText call.");

	std::va_list args;
	char buf[1024];

	va_start(args,str);
	std::vsnprintf(buf, 1024, str, args);   // Avoid buffer overflow.
	va_end(args);
	// Call the real drawing function now that the string is formatted.
	drawText(x, y, std::string(buf));
}

void Font::drawText(float x, float y, const std::string& str) const
{
	if (!isValid()) throw std::logic_error("Invalid Font::drawText call.");

	glBindTexture(GL_TEXTURE_2D, texID_);
	glPushMatrix();
	glTranslated(x,y,0);
	for (std::string::const_iterator i = str.begin(); i != str.end(); ++i) {
		unsigned char ch( *i - SPACE ); // ch-SPACE = DisplayList offset
		// Replace characters outside the valid range with undrawable.
		if(ch > NUM_CHARS) ch = NUM_CHARS-1;   // Last character is 'undrawable'.
		glCallList(listBase_+ch);    // Calculate list to call.
	}

	// Alternative, ignores undrawables (no noticable speed difference).
	//glListBase(listBase_-32);
	//glCallLists(static_cast<int>(std::strlen(buf)), GL_UNSIGNED_BYTE, buf);

	glPopMatrix();
}

std::ostream& Font::out(float x, float y)
{
	// Clear the string and store the draw-position.
	ss_.str("");
	drawX_ = x;
	drawY_ = y;
	return ss_;
}

StreamFlusher Font::end()
{
	drawText(drawX_, drawY_, ss_.str());    // Draw the string.
	ss_.str("");                            // Clear the buffer.
	return StreamFlusher();
}

unsigned int Font::getWidth(const std::string& str) const
{
	if (!isValid()) throw std::logic_error("Invalid Font::calcStringWidth call.");

	unsigned int width = 0;

	// Iterate through widths of each char and accumulate width of string.
	for (std::string::const_iterator i = str.begin(); i < str.end(); ++i)
		width += widths_[static_cast<unsigned int>(*i) - SPACE];

	return width;
}

unsigned int Font::getHeight() const
{
	if (!isValid()) throw std::logic_error("Invalid Font::getHeight call.");

	return height_;
}
