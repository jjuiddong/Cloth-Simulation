//
// 2019-12-13, jjuiddong
// cloth simulation, particle object
//
// reference
//		https://viscomp.alexandra.dk/?p=147
//
// The particle class represents a particle of mass that can move 
// around in 3D space
//
#pragma once



class cParticle
{
public:
	cParticle(const Vector3 &pos);
	cParticle();

	void AddForce(const Vector3 &f);
	void TimeStep();
	void OffsetPos(const Vector3 &v);
	void MakeUnmovable();


public:
	bool m_movable; // can the particle move or not ? used to pin parts of the cloth

	float m_mass; // the mass of the particle (is always 1 in this example)
	Vector3 m_pos; // the current position of the particle in 3D space
	Vector3 m_oldPos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
	Vector3 m_acceleration; // a vector representing the current acceleration of the particle
	// notice, the normal is not unit length
	Vector3 m_accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
};
