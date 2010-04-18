#include <string>
#include <stdexcept>
#include <algorithm>
#include <SOIL.h>
#include <GL/gl.h>

#include "texture.hh"

GLuint load_texture(const char* filename) {
	GLuint handle = SOIL_load_OGL_texture
		(
			filename,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
		);

	/* check for an error during the load process */
	if (0 == handle) throw std::runtime_error(std::string("SOIL couldn't load image ") + filename + std::string(": ") + SOIL_last_result());
	return handle;
}


TextureMap load_textures() {
	TextureMap tmap;
	tmap.insert(std::pair<std::string, GLuint>("background", load_texture("../images/bg.png")));
	tmap.insert(std::pair<std::string, GLuint>("water", load_texture("../images/water.png")));
	tmap.insert(std::pair<std::string, GLuint>("ground", load_texture("../images/ground.png")));
	tmap.insert(std::pair<std::string, GLuint>("ladder", load_texture("../images/ladder.png")));
	tmap.insert(std::pair<std::string, GLuint>("crate", load_texture("../images/crate.png")));
	tmap.insert(std::pair<std::string, GLuint>("powerups", load_texture("../images/powerups.png")));
	tmap.insert(std::pair<std::string, GLuint>("tomato", load_texture("../images/player_1.png")));

	return tmap;
}


float* getTileTexCoords(int tileid, int xtiles, int ytiles, bool horiz_flip, float xoff, float yoff) {
	static CoordArray tc;
	float tilew = 1.0f / xtiles;
	float tileh = 1.0f / ytiles;
	float x = (tileid % xtiles) * tilew + xoff;
	float y = 1.0 - int(tileid / xtiles) * tileh - yoff;
	tc.clear();
	if (horiz_flip) { // Flipped
		float temp[] = { x + tilew, y - tileh,
		                 x + tilew, y,
		                 x, y,
		                 x, y - tileh };
		tc.insert(tc.end(), &temp[0], &temp[8]);
	} else { // Non-flipped
		float temp[] = { x, y - tileh,
		                 x, y,
		                 x + tilew, y,
		                 x + tilew, y - tileh };
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
	glDrawArrays(GL_QUADS, 0, n);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

