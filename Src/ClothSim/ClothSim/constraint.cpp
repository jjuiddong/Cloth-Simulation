
#include "stdafx.h"
#include "constraint.h"


cConstraint::cConstraint(cParticle *p1, cParticle *p2) 
	: p1(p1)
	, p2(p2)
{
	Vector3 vec = p1->pos - p2->pos;
	rest_distance = vec.Length();
}


// This is one of the important methods, where a single constraint 
// between two particles p1 and p2 is solved
// the method is called by Cloth.time_step() many times per frame
void cConstraint::satisfyConstraint() const
{
	Vector3 p1_to_p2 = p2->pos - p1->pos; // vector from p1 to p2
	float current_distance = p1_to_p2.Length(); // current distance between p1 and p2
	Vector3 correctionVector = p1_to_p2 * (1.f - rest_distance / current_distance); // The offset vector that could moves p1 into a distance of rest_distance to p2
	Vector3 correctionVectorHalf = correctionVector * 0.5f; // Lets make it half that length, so that we can move BOTH p1 and p2.
	p1->offsetPos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
	p2->offsetPos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
}
