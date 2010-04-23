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
    		cx = float(x / 16.0f);
			cy = float(y / 16.0f);

			glNewList(font_base + (y*16+x), GL_COMPILE);
				glBegin(GL_QUADS);
					glTexCoord2f(cx, 1 - cy - 0.0625f);
					glVertex2i(0, 16);
					glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);
					glVertex2i(16, 16);
					glTexCoord2f(cx + 0.0625f, 1 - cy);
					glVertex2i(16, 0);
					glTexCoord2f(cx, 1 - cy);
					glVertex2i(0, 0);
				glEnd();
				glTranslated(10, 0, 0);
			glEndList();
		}
}

void drawText(GLuint texture, Color color, const float x, const float y, const std::string& text)
{
	glEnable(GL_TEXTURE_2D);
	glColor4f(color.r, color.g, color.b, color.a);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glPushMatrix();
	glTranslatef(x, y, 0);
 	glListBase(font_base);
	glCallLists(text.length(), GL_UNSIGNED_BYTE, text.c_str());
	glPopMatrix();
	// Restore blend settings
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

