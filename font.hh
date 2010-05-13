#pragma once
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

#include <GL/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdarg>
#include <sstream>
#include <boost/noncopyable.hpp>

/// FreeType library container
class FTLibraryContainer {
  public:
	FTLibraryContainer() {
		if (FT_Init_FreeType(&m_library))
			throw std::runtime_error("Could not initialize FreeType2 library.");
	}
	~FTLibraryContainer() { FT_Done_FreeType(m_library); }
	FT_Library& getLibrary() { return m_library; }
  private:
	static FT_Library m_library;
};


class StreamFlusher { };
std::ostream& operator<<(std::ostream& os, const StreamFlusher& rhs);

/// Font class
class Font: public boost::noncopyable
{
  public:
	Font(const std::string& filename = "", unsigned int size = 10) :
		texID_(0), listBase_(0),    // Initalize GL variables to zero
		widths_(NUM_CHARS),         // Make room for character widths
		height_(0), drawX_(0), drawY_(0)
		{ if (filename != "") open(filename, size); }

	~Font() { release(); }

	void open(const std::string& filename, unsigned int size);
	void release();

	bool isValid() const { return glIsTexture(texID_) == GL_TRUE; }

	void drawText(float x, float y, const char *str, ...) const;
	void drawText(float x, float y, const std::string& str) const;

	std::ostream& out(float x, float y);
	StreamFlusher end();

	unsigned int getWidth(const std::string& str) const;
	unsigned int getHeight() const;

  private:
	// Font data
	unsigned int texID_;
	unsigned int listBase_;
	std::vector<unsigned char> widths_;
	unsigned char height_;
	// Stream drawing stuff
	std::ostringstream ss_;
	float drawX_;
	float drawY_;

	static const unsigned int SPACE = 32;
	static const unsigned int NUM_CHARS = 96;

	static FTLibraryContainer library_;
};
