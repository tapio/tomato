#pragma once

#include <map>
#include <string>
#include <vector>
#include <GL/gl.h>

typedef std::map<std::string, GLuint> TextureMap;
typedef std::vector<float> CoordArray;

const static float tex_square[] = { 0.0f, 0.0f,
	                                0.0f, 1.0f,
	                                1.0f, 0.0f,
	                                1.0f, 1.0f };


/// Load a single texture
GLuint load_texture(const char* filename);

/// Load all textures used by the program
TextureMap load_textures();

/// Compose texture coordinate array from a tile index
float* getTileTexCoords(int tileid, int tilesize, int texsize);

/// Draw a given vertex array with triangle strip
void drawVertexArray(const float* v_a, const float* t_a, GLuint n, GLuint tex);
