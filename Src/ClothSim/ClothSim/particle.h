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

	void addForce(const Vector3 &f);
	void timeStep();
	void offsetPos(const Vector3 &v);
	void makeUnmovable();


public:
	bool movable; // can the particle move or not ? used to pin parts of the cloth

	float mass; // the mass of the particle (is always 1 in this example)
	Vector3 pos; // the current position of the particle in 3D space
	Vector3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
	Vector3 acceleration; // a vector representing the current acceleration of the particle
	// notice, the normal is not unit length
	Vector3 accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
};
