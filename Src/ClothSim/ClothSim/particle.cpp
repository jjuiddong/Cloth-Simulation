
#include "stdafx.h"
#include "particle.h"


cParticle::cParticle(const Vector3 &pos)
	: pos(pos)
	, old_pos(pos)
	, acceleration(Vector3(0, 0, 0))
	, mass(1)
	, movable(true)
	, accumulated_normal(Vector3(0, 0, 0))
{
}

cParticle::cParticle() 
{
}


void cParticle::addForce(const Vector3 &f)
{
	acceleration += f / mass;
}


//  This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
//   The method is called by Cloth.time_step()
//   Given the equation "force = mass * acceleration" the next position is found through verlet integration
void cParticle::timeStep()
{
	if (movable)
	{
		Vector3 temp = pos;
		pos = pos + (pos - old_pos)*(1.0f - DAMPING) 
			+ acceleration * TIME_STEPSIZE2;
		old_pos = temp;

		// acceleration is reset since it HAS been translated 
		// into a change in position (and implicitely into velocity)	
		acceleration = Vector3(0, 0, 0);
	}
}


void cParticle::offsetPos(const Vector3 &v)
{ 
	if (movable)
		pos += v; 
}


void cParticle::makeUnmovable() 
{ 
	movable = false; 
}
