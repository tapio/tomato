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
	tmap.insert(std::pair<std::string, GLuint>("tomato", load_texture("../images/tomaatti.png")));

	return tmap;
}


float* getTileTexCoords(int tileid, int tilesize, int texsize) {
	static CoordArray tc;
	int tiles_per_row = texsize / tilesize;
	float tiletexsize = (float)tilesize / texsize;
	float x = (tileid % tiles_per_row) * tiletexsize;
	float y = int(tileid / tiles_per_row) * tiletexsize;
	float temp[] = { x, y,
	                 x, y + tiletexsize,
	                 x + tiletexsize, y,
	                 x + tiletexsize, y + tiletexsize };
	tc.clear();
	tc.insert(tc.end(), &temp[0], &temp[8]);
	return &tc[0];
}
