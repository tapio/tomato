#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <GL/gl.h>
#include <Box2D.h>

#include "texture.hh"
#include "util.hh"

struct WorldElement {
	WorldElement(float w, float h, GLuint tex, GLuint tile, int tsize): w(w), h(h), texture(tex), tileid(tile), tilesize(tsize), seed(randint(1000))
	{ }
	// ARGH, horibble spaghetti below
	void draw() const {
		srand(seed);
		float x = getX(), y = getY();
		float hw = getW() * 0.5, hh = getH() * 0.5;
		//float xmax = w > h ? w / h : 1.0f;
		//float ymax = h > w ? h / w : 1.0f;
		CoordArray v_arr, t_arr;
		if (w > h) { // Horizontal
			// Left side (beginning)
			float* tc = getTileTexCoords(0, 8, 1);
			t_arr.insert(t_arr.end(), tc, tc+8);
			float vc_beg[] = { x - hw, y - hh + tilesize,
							   x - hw, y - hh,
							   x - hw + tilesize*0.5f, y - hh,
							   x - hw + tilesize*0.5f, y - hh + tilesize };
			v_arr.insert(v_arr.end(), &vc_beg[0], &vc_beg[8]);
			// Middle
			for (int i = 0; i < w-1; i++) {
				float xx = x - hw + tilesize * 0.5f + i * tilesize;
				float yy = y - hh;
				float vc_mid[] = { xx, yy + tilesize,
								   xx, yy,
								   xx + tilesize, yy,
								   xx + tilesize, yy + tilesize };
				v_arr.insert(v_arr.end(), &vc_mid[0], &vc_mid[8]);
				tc = getTileTexCoords(randint(0,2), 4, 1, false, 1.0f/8.0f); // xoffset for the left ending
				t_arr.insert(t_arr.end(), tc, tc+8);
			}
			// Right side (end)
			tc = getTileTexCoords(7, 8, 1);
			t_arr.insert(t_arr.end(), tc, tc+8);
			float vc_end[] = { x + hw - tilesize*0.5f, y - hh + tilesize,
							   x + hw - tilesize*0.5f, y - hh,
							   x + hw, y - hh,
							   x + hw, y - hh + tilesize };
			v_arr.insert(v_arr.end(), &vc_end[0], &vc_end[8]);
		} else { // Vertical
			// Top (beginning)
			float* tc = getTileTexCoords(0, 1, 8);
			t_arr.insert(t_arr.end(), tc, tc+8);
			float vc_beg[] = { x - hw, y - hh + tilesize*0.5f,
							   x - hw, y - hh,
							   x - hw + tilesize, y - hh,
							   x - hw + tilesize, y - hh + tilesize*0.5f };
			v_arr.insert(v_arr.end(), &vc_beg[0], &vc_beg[8]);
			// Middle
			for (int j = 0; j < h-1; j++) {
				float xx = x - hw;
				float yy = y - hh + tilesize * 0.5f + j * tilesize;
				float vc_mid[] = { xx, yy + tilesize,
								   xx, yy,
								   xx + tilesize, yy,
								   xx + tilesize, yy + tilesize };
				v_arr.insert(v_arr.end(), &vc_mid[0], &vc_mid[8]);
				tc = getTileTexCoords(randint(0,2), 1, 4, false, 0.0f, 1.0f/8.0f); // yoffset for the half-top
				t_arr.insert(t_arr.end(), tc, tc+8);
			}
			// Bottom (end)
			tc = getTileTexCoords(7, 1, 8);
			t_arr.insert(t_arr.end(), tc, tc+8);
			float vc_end[] = { x + hw - tilesize, y + hh,
							   x + hw - tilesize, y + hh - tilesize*0.5f,
							   x + hw, y + hh - tilesize*0.5f,
							   x + hw, y + hh };
			v_arr.insert(v_arr.end(), &vc_end[0], &vc_end[8]);
		}
		// Draw
		drawVertexArray(&v_arr[0], &t_arr[0], v_arr.size()/2, texture);
	}

	float getX() const { return body->GetPosition().x; };
	float getY() const { return body->GetPosition().y; };
	float getW() const { return w * tilesize; };
	float getH() const { return h * tilesize; };

	b2Body* body;
	float w, h;
	GLuint texture;
	GLuint tileid;
	int tilesize;
	int seed;

};

struct Platform: public WorldElement {
	Platform(int w, GLuint tex, GLuint tile, int tsize): WorldElement(w, 1, tex, tile, tsize) {}
};

struct Ladder: public WorldElement {
	Ladder(int h, GLuint tex, GLuint tile, int tsize): WorldElement(1, h, tex, tile, tsize) {}
};

class Player;

typedef Player Actor;
typedef std::vector<Actor> Actors;
typedef std::vector<Platform> Platforms;
typedef std::vector<Ladder> Ladders;

class World {
  public:
	World(int width, int height, TextureMap& tm): world(b2Vec2(0.0f, 15.0f), true), w(width), h(height), tilesize(32), water_height(64)
	{
		float hw = w*0.5, hh = h*0.5;

		// Define the border bodies
		b2BodyDef borderBodyDef;
		borderBodyDef.position.Set(hw, hh);
		b2Body* borderBody = world.CreateBody(&borderBodyDef);

		// Define the border shapes
		b2PolygonShape borderBoxLeft, borderBoxRight, borderBoxTop, borderBoxBottom;
		borderBoxLeft.SetAsEdge(b2Vec2(-hw,-hh), b2Vec2(-hw,hh));
		borderBoxRight.SetAsEdge(b2Vec2(hw,-hh), b2Vec2(hw,hh));
		borderBoxTop.SetAsEdge(b2Vec2(-hw,-hh), b2Vec2(hw,-hh));
		borderBoxBottom.SetAsEdge(b2Vec2(-hw,hh), b2Vec2(hw,hh));

		// Add the border fixtures to the body
		borderBody->CreateFixture(&borderBoxLeft, 0.0f);
		borderBody->CreateFixture(&borderBoxRight, 0.0f);
		borderBody->CreateFixture(&borderBoxTop, 0.0f);
		borderBody->CreateFixture(&borderBoxBottom, 0.0f);

		// Create water
		b2BodyDef waterBodyDef;
		waterBodyDef.position.Set(hw, h - water_height*0.5f);
		b2Body* waterBody = world.CreateBody(&waterBodyDef);
		b2PolygonShape waterBox;
		waterBox.SetAsBox(w*0.5f, water_height*0.5f);
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &waterBox;
		fixtureDef.isSensor = true; // No collision response
		waterBody->CreateFixture(&fixtureDef);

		texture_background = tm.find("background")->second;
		texture_water = tm.find("water")->second;
		texture_ground = tm.find("ground")->second;
		texture_ladder = tm.find("ladder")->second;

		generate();
	}

	void addActor(float x, float y, GLuint tex);
	void addPlatform(float x, float y, float w);
	void addLadder(float x, float y, float h);

	void generate();

	void update();

	void draw() const;

	b2World& getWorld() { return world; }
	Actors& getActors() { return actors; }

  private:

	b2World world;
	int w;
	int h;
	int tilesize;
	int water_height;
	GLuint texture_background;
	GLuint texture_water;
	GLuint texture_ground;
	GLuint texture_ladder;
	Actors actors;
	Platforms platforms;
	Ladders ladders;

};
