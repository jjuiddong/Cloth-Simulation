
#include "stdafx.h"
#include "particle.h"


cParticle::cParticle(const Vector3 &pos)
	: m_pos(pos)
	, m_oldPos(pos)
	, m_acceleration(Vector3(0, 0, 0))
	, m_mass(1.f)
	, m_movable(true)
	, m_accumulated_normal(Vector3(0, 0, 0))
{
}

cParticle::cParticle() 
{
}


void cParticle::AddForce(const Vector3 &f)
{
	m_acceleration += f / m_mass;
}


// This is one of the important methods, where the time is progressed 
// a single step size (TIME_STEPSIZE) The method is called by Cloth.TimeStep()
// Given the equation "force = mass * acceleration" the next position is 
// found through verlet integration
void cParticle::TimeStep(const float deltaSeconds)
{
	// how much to damp the cloth simulation each frame
	const float DAMPING = 0.01f;

	if (m_movable)
	{
		Vector3 temp = m_pos;
		m_pos = m_pos + (m_pos - m_oldPos)*(1.0f - DAMPING)
			+ m_acceleration * deltaSeconds * deltaSeconds;
		m_oldPos = temp;

		// acceleration is reset since it HAS been translated 
		// into a change in position (and implicitely into velocity)	
		m_acceleration = Vector3(0, 0, 0);
	}
}


void cParticle::OffsetPos(const Vector3 &v)
{ 
	if (m_movable)
		m_pos += v; 
}


void cParticle::MakeUnmovable() 
{ 
	m_movable = false;
}
