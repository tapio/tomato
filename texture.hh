#pragma once
#include <SOIL.h>
#include <GL/gl.h>
#include <map>

typedef std::map<std::string, GLuint> TextureMap;

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
	tmap.insert(std::pair<std::string, GLuint>("ground", load_texture("../images/ground.png")));
	tmap.insert(std::pair<std::string, GLuint>("tomato", load_texture("../images/tomaatti.png")));

	return tmap;
}
