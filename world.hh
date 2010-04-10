#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <GL/gl.h>
#include <Box2D.h>

#include "texture.hh"

struct WorldElement {
	WorldElement(float w, float h, GLuint tex, GLuint tile, int tsize): w(w), h(h), texture(tex), tileid(tile), tilesize(tsize)
	{ }
	// ARGH, horibble spaghetti below
	void draw() const {
		float x = getX(), y = getY();
		float hw = getW() * 0.5, hh = getH() * 0.5;
		//float xmax = w > h ? w / h : 1.0f;
		//float ymax = h > w ? h / w : 1.0f;
		CoordArray v_arr, t_arr;
		if (w > h) {
			float tc_beg[] = { 0.00f,0.0f, 0.00f,1.0f, 0.25f,0.0f, 0.25f,1.0f };
			float tc_mid[] = { 0.25f,0.0f, 0.25f,1.0f, 0.75f,0.0f, 0.75f,1.0f };
			float tc_end[] = { 0.75f,0.0f, 0.75f,1.0f, 1.00f,0.0f, 1.00f,1.0f };
			float vc_beg[] = { x - hw, y - hh + tilesize,
							   x - hw, y - hh,
							   x - hw + tilesize*0.5f, y - hh + tilesize,
							   x - hw + tilesize*0.5f, y - hh };
			float vc_end[] = { x + hw - tilesize*0.5f, y - hh + tilesize,
							   x + hw - tilesize*0.5f, y - hh,
							   x + hw, y - hh + tilesize,
							   x + hw, y - hh };
			// Beginning
			v_arr.insert(v_arr.end(), &vc_beg[0], &vc_beg[8]);
			t_arr.insert(t_arr.end(), &tc_beg[0], &tc_beg[8]);
			// Middle
			for (int i = 0; i < w-1; i++) {
				float xx = x - hw + tilesize * 0.5f + i * tilesize;
				float yy = y - hh; //+ tilesize * 0.5f + j * tilesize;
				float vc_mid[] = { xx, yy + tilesize,
								   xx, yy,
								   xx + tilesize, yy + tilesize,
								   xx + tilesize, yy };
				v_arr.insert(v_arr.end(), &vc_mid[0], &vc_mid[8]);
				t_arr.insert(t_arr.end(), &tc_mid[0], &tc_mid[8]);
			}
			// End
			v_arr.insert(v_arr.end(), &vc_end[0], &vc_end[8]);
			t_arr.insert(t_arr.end(), &tc_end[0], &tc_end[8]);
		} else {
			float tc_beg[] = { 0.0f,0.00f, 0.0f,0.25f, 1.0f,0.00f, 1.0f,0.25f };
			float tc_mid[] = { 0.0f,0.25f, 0.0f,0.75f, 1.0f,0.25f, 1.0f,0.75f };
			float tc_end[] = { 0.0f,0.75f, 0.0f,1.00f, 1.0f,0.75f, 1.0f,1.00f };
			float vc_beg[] = { x - hw, y - hh + tilesize,
							   x - hw, y - hh,
							   x - hw + tilesize, y - hh + tilesize,
							   x - hw + tilesize, y - hh };
			float vc_end[] = { x + hw, y - hh + tilesize,
							   x + hw, y - hh,
							   x + hw - tilesize, y - hh + tilesize,
							   x + hw - tilesize, y - hh };
			// Beginning
			v_arr.insert(v_arr.end(), &vc_beg[0], &vc_beg[8]);
			t_arr.insert(t_arr.end(), &tc_beg[0], &tc_beg[8]);
			// Middle
			for (int j = 0; j < h-1; j++) {
				float xx = x - hw;
				float yy = y - hh + tilesize * 0.5f + j * tilesize;
				float vc_mid[] = { xx, yy + tilesize,
								   xx, yy,
								   xx + tilesize, yy + tilesize,
								   xx + tilesize, yy };
				v_arr.insert(v_arr.end(), &vc_mid[0], &vc_mid[8]);
				t_arr.insert(t_arr.end(), &tc_mid[0], &tc_mid[8]);
			}
			// End
			v_arr.insert(v_arr.end(), &vc_end[0], &vc_end[8]);
			t_arr.insert(t_arr.end(), &tc_end[0], &tc_end[8]);
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
	World(int width, int height, TextureMap& tm): world(b2Vec2(0.0f, 15.0f), true), w(width), h(height)
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

		texture_background = tm.find("background")->second;
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
	GLuint texture_background;
	GLuint texture_ground;
	GLuint texture_ladder;
	Actors actors;
	Platforms platforms;
	Ladders ladders;

};
