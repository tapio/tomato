#pragma once

#include <vector>
#include <GL/gl.h>
#include <Box2D.h>

struct Platform {
	Platform(float w, float h, GLuint tex, GLuint tile, int tsize): w(w), h(h), texture(tex), tileid(tile), tilesize(tsize)
	{ }

	void draw() const {
		float x = getX(), y = getY();
		float hw = getW() * 0.5, hh = getH() * 0.5;
		//float xmax = w > h ? w / h : 1.0f;
		//float ymax = h > w ? h / w : 1.0f;
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(x - hw, y - hh + tilesize);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(x - hw, y - hh);
			glTexCoord2f(0.25f, 0.0f);
			glVertex2f(x - hw + tilesize*0.5f, y - hh + tilesize);
			glTexCoord2f(0.25f, 1.0f);
			glVertex2f(x - hw + tilesize*0.5f, y - hh);
			for (int i = 0; i < w-1; i++) {
				float xx = x - hw + tilesize * 0.5f + i * tilesize;
				float yy = y - hh; //+ tilesize * 0.5f + j * tilesize;
				glTexCoord2f(0.25f, 0.0f);
				glVertex2f(xx, yy + tilesize);
				glTexCoord2f(0.25f, 1.0f);
				glVertex2f(xx, yy);
				glTexCoord2f(0.75f, 0.0f);
				glVertex2f(xx + tilesize, yy + tilesize);
				glTexCoord2f(0.75f, 1.0f);
				glVertex2f(xx + tilesize, yy);
			}
			glTexCoord2f(0.75f, 0.0f);
			glVertex2f(x + hw - tilesize*0.5f, y - hh + tilesize);
			glTexCoord2f(0.75f, 1.0f);
			glVertex2f(x + hw - tilesize*0.5f, y - hh);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(x + hw, y - hh + tilesize);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(x + hw, y - hh);
		glEnd();
		glDisable(GL_TEXTURE_2D);
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

typedef std::vector<Platform> Platforms;

class Player;

typedef Player Actor;

class World {
  public:
	World(int width, int height, GLuint tex): world(b2Vec2(0.0f, 2.0f), true), w(width), h(height), texture(tex)
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

		generate();
	}

	void addActor(Actor* actor);

	void addPlatform(float x, float y, float w);

	void generate();

	void update();

	void draw() const;

	b2World& getWorld() { return world; }

  private:

	b2World world;
	int w;
	int h;
	GLuint texture;
	typedef std::vector<Actor*> Actors;
	Actors actors;
	Platforms platforms;

};
