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

			glNewList(font_base + (y*16+x), GL_COMPILE);
				glBegin(GL_QUADS);
					glTexCoord2f(cx, 1 - cy - 0.0625);
					glVertex2i(0, 16);
					glTexCoord2f(cx + 0.0625, 1 - cy - 0.0625);
					glVertex2i(16, 16);
					glTexCoord2f(cx + 0.0625, 1 - cy);
					glVertex2i(16, 0);
					glTexCoord2f(cx, 1 - cy);
					glVertex2i(0, 0);
				glEnd();
				glTranslated(10, 0, 0);	// 10 really advance, fix when converting to arrays
			glEndList();
		}
}

void drawText(GLuint texture, Color color, const float x, const float y, const std::string& text, FontAlign align)
{
	float advance = 10.0;
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
	
	glPushMatrix();
	glTranslatef(xx, yy, 0);
 	glListBase(font_base);
	glCallLists(text.length(), GL_UNSIGNED_BYTE, text.c_str());
	glPopMatrix();
	// Restore blend settings
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

