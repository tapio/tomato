#include <GL/gl.h>

#include "util.hh"


void drawVertexArray(const float* v_a, const float* t_a, GLuint n, GLuint tex) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, t_a);
	glVertexPointer(2, GL_FLOAT, 0, v_a);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, n);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
}
