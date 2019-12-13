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
	void satisfyConstraint() const;


public:
	// the two particles that are connected through this constraint
	cParticle *p1, *p2;

private:
	// the length between particle p1 and p2 in rest configuration
	float rest_distance;
};
