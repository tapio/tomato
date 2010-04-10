#include <string>
#include <stdexcept>
#include <SOIL.h>
#include <GL/gl.h>

#include "texture.hh"

GLuint load_texture(const char* filename) {
	GLuint handle = SOIL_load_OGL_texture
		(
			filename,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
		);

	/* check for an error during the load process */
	if (0 == handle) throw std::runtime_error(std::string("SOIL couldn't load image ") + filename + std::string(": ") + SOIL_last_result());
	return handle;
}


TextureMap load_textures() {
	TextureMap tmap;
	tmap.insert(std::pair<std::string, GLuint>("background", load_texture("../images/bg.png")));
	tmap.insert(std::pair<std::string, GLuint>("ground", load_texture("../images/ground.png")));
	tmap.insert(std::pair<std::string, GLuint>("tomato", load_texture("../images/player_1.png")));

	return tmap;
}


const float* getTileTexCoords(int tileid, int tilesize, int texsize, bool horiz_flip) {
	static CoordArray tc;
	int tiles_per_row = texsize / tilesize;
	float tiletexsize = (float)tilesize / texsize;
	float x = (tileid % tiles_per_row) * tiletexsize;
	float y = 1.0 - int(tileid / tiles_per_row) * tiletexsize;
	tc.clear();
	if (horiz_flip) {
		float temp[] = { x + tiletexsize, y - tiletexsize,
		                 x + tiletexsize, y,
		                 x, y - tiletexsize,
		                 x, y };
		tc.insert(tc.end(), &temp[0], &temp[8]);
	} else {
		float temp[] = { x, y - tiletexsize,
		                 x, y,
		                 x + tiletexsize, y - tiletexsize,
		                 x + tiletexsize, y };
		tc.insert(tc.end(), &temp[0], &temp[8]);
	}
	return &tc[0];
}


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
