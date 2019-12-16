//
// 2019-12-13, jjuiddong
// cloth simulation, constraints object
//
// reference
//		- https://viscomp.alexandra.dk/?p=147
//
#pragma once


class cConstraint
{

public:
	cConstraint(cParticle *p1, cParticle *p2);
	void SatisfyConstraint() const;


public:
	// the two particles that are connected through this constraint
	cParticle *m_p1, *m_p2;
	// the length between particle p1 and p2 in rest configuration
	float m_rest_distance;
};
