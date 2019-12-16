//
// 2019-12-13, jjuiddong
// cloth simulation, cloth object
//
// reference
//		https://viscomp.alexandra.dk/?p=147
//
#pragma once


struct sCBClothParameter
{
	XMVECTOR deltaSeconds;
	XMVECTOR force;
};


struct sConstraints
{
	//Vector4 index; // x=paritcle 1 index, y=particle2 index, z=rest_distance
	int p1; // particle1 index
	int p2; // particle2 index
	float rest_distance;
};

struct sConstraints2
{
	int p1;
	int p2;
};


// SatisfyConstraint Process
struct sParticle1
{
	//Vector4 movMass; // x=m_movable, y=m_mass
	float movable;
	float mass;
	Vector4 pos;
	Vector4 oldPos;
	Vector4 acceleration;
	Vector4 accumulated_normal;
};


class cCloth
{
public:
	cCloth();
	virtual ~cCloth();

	bool Init(graphic::cRenderer &renderer
		, float width, float height, int col, int row, int iterationCount);
	void DrawShaded(graphic::cRenderer &renderer);
	void TimeStep(graphic::cRenderer &renderer, const float deltaSeconds);
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


public:
	// total number of particles is col * row
	int m_col; // number of particles in "width" direction
	int m_row; // number of particles in "height" direction
	vector<cParticle> m_particles; // all particles that are part of this cloth
	vector<cConstraint> m_constraints; // alle constraints between particles as part of this cloth
	vector<WORD> m_indices; // triangle index array

	// how many iterations of constraint satisfaction each frame (more is rigid, less is soft)
	int m_iterationCount;

	int m_posOffset;
	int m_normOffset;
	int m_colorOffset;
	int m_vertexStride;
	graphic::cVertexBuffer m_vtxBuff;
	Vector3 m_force;

	// compute shader optimize
	graphic::cShader11 m_shader; // compute shader
	graphic::cShareBuffer m_contraintsBuff; // compute shader input buffer
	graphic::cShareBuffer m_particleBuff;
	graphic::cShareBuffer m_resultBuff;
	graphic::cShareBuffer m_copyBuff;
	graphic::cConstantBuffer<sCBClothParameter> m_cbParameter;
};
