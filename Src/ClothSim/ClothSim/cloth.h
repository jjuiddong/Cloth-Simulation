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
	void DrawShaded(graphic::cRenderer &renderer);
	void TimeStep();
	void AddForce(const Vector3 &direction);
	void WindForce(const Vector3 &direction);
	void BallCollision(const Vector3 &center, const float radius);
	void DoFrame();
	void Clear();


protected:
	cParticle* GetParticle(int x, int y);
	void MakeConstraint(cParticle *p1, cParticle *p2);
	Vector3 CalcTriangleNormal(cParticle *p1, cParticle *p2, cParticle *p3);
	void AddWindForcesForTriangle(cParticle *p1, cParticle *p2, cParticle *p3
		, const Vector3 direction);
	void DrawTriangle(BYTE *p
		, cParticle *p1, cParticle *p2, cParticle *p3, const Vector3 color);
	int GetIndex(int x, int y);


private:
	// total number of particles is m_num_particles_width * m_num_particles_height
	int m_num_particles_width; // number of particles in "width" direction
	int m_num_particles_height; // number of particles in "height" direction

	vector<cParticle> m_particles; // all particles that are part of this cloth
	vector<cConstraint> m_constraints; // alle constraints between particles as part of this cloth

	int m_posOffset;
	int m_normOffset;
	int m_colorOffset;
	int m_vertexStride;
	graphic::cVertexBuffer m_vtxBuff;
	graphic::cIndexBuffer m_idxBuff;
};
