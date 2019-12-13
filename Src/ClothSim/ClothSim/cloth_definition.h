//
// 2019-12-13, jjuiddong
// cloth simulation global definition
//
// reference
//		- https://viscomp.alexandra.dk/?p=147
//
#pragma once


// Some physics constants
#define DAMPING 0.01f // how much to damp the cloth simulation each frame
#define TIME_STEPSIZE2 0.5f * 0.5f // how large time step each particle takes each frame
#define CONSTRAINT_ITERATIONS 15 // how many iterations of constraint satisfaction each frame (more is rigid, less is soft)

