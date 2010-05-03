#include "font.hh"

GLuint font_base;

void buildFonts()
{
	float	cx;
	float	cy;
    int x, y;
    font_base = glGenLists(256);
    for(y = 0; y < 16; y++)
        for(x = 0; x < 16; x++)
        {
    		cx = float(x / 16.0);
			cy = float(y / 16.0);

			glNewList(font_base + (y*16+x)-1, GL_COMPILE);
				glBegin(GL_QUADS);
					glTexCoord2f(cx, 1 - cy - 0.0625 + 2*0.0039);
					glVertex2i(0, 16-2);
					glTexCoord2f(cx + 0.0625, 1 - cy - 0.0625 + 2*0.0039);
					glVertex2i(16, 16-2);
					glTexCoord2f(cx + 0.0625, 1 - cy - 0.0039);
					glVertex2i(16, 0);
					glTexCoord2f(cx, 1 - cy - 0.0039);
					glVertex2i(0, 0);
				glEnd();
			glEndList();
		}
}

void drawText(GLuint texture, Color color, const float x, const float y, const std::string& text, FontAlign align)
{
	float advance = 10.0;
	float height = 16.0;
	float xx = 0, yy = 0;
	if(align == ALIGN_LEFT) {
		xx = x; yy = y;
	}
	else if(align == ALIGN_CENTER) {
		float halflen = text.length()*advance*0.5;
		xx = x - halflen; yy = y; //- 8;
	}
	else if(align == ALIGN_RIGHT) {
		float textlen = text.length()*advance;
		xx = x - textlen; yy = y;
	}

	glEnable(GL_TEXTURE_2D);
	glColor4f(color.r, color.g, color.b, color.a);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glPushMatrix();
	glTranslatef(xx, yy, 0);
 	
 	const unsigned char* str = (unsigned char*)text.c_str();
 	for(unsigned int i=0; i<text.length(); ++i) {
		// Newline
		if(str[i] == '\n') { glTranslated(0, height, 0); i++; }
		// 	ISO-8859-1 emulation
		if(str[i] == 0xC3) {
			i++;
			if(str[i] == 0x84) { glCallList(0xC4); } // Ä
			else if(str[i] == 0xA4) { glCallList(0xE4); } // ä
			else if(str[i] == 0x96) { glCallList(0xD6); } // Ö
			else if(str[i] == 0xB6) { glCallList(0xF6); } // ö
			glTranslated(advance, 0.0, 0.0);
			continue;
		}
		glCallList(str[i]);
		glTranslated(advance, 0.0, 0.0);
	}
	glPopMatrix();
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

