#include <GL/gl.h>
#include <Box2D.h>

#include "entity.hh"

struct WorldElement: public Entity {
	WorldElement(float w, float h, GLuint tex, GLuint tile, int tsize): Entity(tex, 0),
	  w(w), h(h), tileid(tile), tilesize(tsize)
	{ if (w != h && w != 0 && h != 0 && getBody()) buildVertices(); }
	// ARGH, horibble spaghetti below
	void buildVertices() {
		float x = getX(), y = getY();
		float hw = getW() * 0.5, hh = getH() * 0.5;
		//float xmax = w > h ? w / h : 1.0f;
		//float ymax = h > w ? h / w : 1.0f;
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
	}

	virtual void draw() const {
		drawVertexArray(&v_arr[0], &t_arr[0], v_arr.size()/2, texture);
	}

	virtual SerializedEntity serialize() const {
		return SerializedEntity(getX(), getY(), w, h);
	}

	virtual void unserialize(std::string data) {
		SerializedEntity* se = reinterpret_cast<SerializedEntity*>(&data[0]);
		getBody()->SetTransform(b2Vec2(se->x, se->y), 0);
		w = se->vx; h = se->vy;
	}

	float getW() const { return w * tilesize; };
	float getH() const { return h * tilesize; };

	CoordArray v_arr, t_arr;
	float w, h;
	GLuint tileid;
	int tilesize;

};

struct Platform: public WorldElement {
	Platform(int w, GLuint tex, GLuint tile, int tsize): WorldElement(w, 1, tex, tile, tsize) {}
};

struct Ladder: public WorldElement {
	Ladder(int h, GLuint tex, GLuint tile, int tsize): WorldElement(1, h, tex, tile, tsize) {}
};

struct Crate: public WorldElement {
	Crate(GLuint tex, GLuint tile, int tsize):  WorldElement(1, 1, tex, tile, tsize) {}
	void draw() const {
		float x = getX(), y = getY();
		float hw = getW() * 0.5, hh = getH() * 0.5;
		glPushMatrix();
		glTranslatef(x, y, 0);
		glRotatef(getBody()->GetAngle() * 180.0 / PI, 0, 0, 1);
		float vc[] = { -hw,  hh,
			           -hw, -hh,
			            hw, -hh,
			            hw,  hh };
		drawVertexArray(&vc[0], &tex_square[0], 4, texture);
		glPopMatrix();
	}
	virtual SerializedEntity serialize() const { return Entity::serialize(); }
	virtual void unserialize(std::string data) { Entity::unserialize(data); }
};

struct Bridge: public WorldElement {
	Bridge(GLuint tex, GLuint tile, int tsize):
	  WorldElement(0, 0, tex, tile, tsize)
	{

	}

	void draw() const {
		glColor4f(0.6f, 0.3f, 0.1f, 1.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINE_STRIP);
		//b2RevoluteJoint* j = dynamic_cast<b2RevoluteJoint*>(bodies.front()->GetJointList()->joint);
		for (std::vector<b2Body*>::const_iterator it = bodies.begin(); it != bodies.end(); ++it) {
			//b2Fixture* f = (*it)->GetFixtureList();
			//b2PolygonShape* s = dynamic_cast<b2PolygonShape*>(f->GetShape());
			//glBegin(GL_QUADS);
				//for (int i = 0; i < 4; ++i) {
					//b2Vec2 v = s->GetVertex(i);
					//v += (*it)->GetPosition();
					//glVertex2f(v.x, v.y);
				//}
			//glEnd();
			b2Vec2 v = (*it)->GetWorldCenter();
			glVertex2f(v.x, v.y);
		}
		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	std::vector<b2Body*> bodies;
};


typedef std::vector<Platform> Platforms;
typedef std::vector<Ladder> Ladders;
typedef std::vector<Crate> Crates;
typedef std::vector<Bridge> Bridges;
