//
// 2019-12-13, jjuiddong
// cloth simulation, cloth object
//
// reference
//		https://viscomp.alexandra.dk/?p=147
//
#pragma once


class cCloth
{
public:
	cCloth();
	virtual ~cCloth();

	bool Init(graphic::cRenderer &renderer
		, float width, float height, int num_particles_width0, int num_particles_height0);
	void drawShaded(graphic::cRenderer &renderer);
	void timeStep();
	void addForce(const Vector3 &direction);
	void windForce(const Vector3 &direction);
	void ballCollision(const Vector3 &center, const float radius);
	void doFrame();
	void Clear();


protected:
	cParticle* getParticle(int x, int y);
	void makeConstraint(cParticle *p1, cParticle *p2);
	Vector3 calcTriangleNormal(cParticle *p1, cParticle *p2, cParticle *p3);
	void addWindForcesForTriangle(cParticle *p1, cParticle *p2, cParticle *p3
		, const Vector3 direction);
	void drawTriangle(BYTE *p
		, cParticle *p1, cParticle *p2, cParticle *p3, const Vector3 color);
	int getIndex(int x, int y);


private:
	int m_num_particles_width; // number of particles in "width" direction
	int m_num_particles_height; // number of particles in "height" direction
	// total number of particles is m_num_particles_width * m_num_particles_height

	vector<cParticle> m_particles; // all particles that are part of this cloth
	vector<cConstraint> m_constraints; // alle constraints between particles as part of this cloth

	int m_posOffset;
	int m_normOffset;
	int m_colorOffset;
	int m_vertexStride;
	graphic::cVertexBuffer m_vtxBuff;
	graphic::cIndexBuffer m_idxBuff;
};
