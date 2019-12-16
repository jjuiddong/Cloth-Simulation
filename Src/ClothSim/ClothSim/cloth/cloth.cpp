
#include "stdafx.h"
#include "cloth.h"


cCloth::cCloth()
	: m_iterationCount(15)
{
}

cCloth::~cCloth()
{
	Clear();
}


// This is a important constructor for the entire system of particles and constraints
// col: num particles width
// row: num particles height
bool cCloth::Init(graphic::cRenderer &renderer
	, float width, float height, int col, int row
	, int iterationCount)
{
	m_col = col;
	m_row = row;
	m_iterationCount = iterationCount;

	// create vertex, index buffer
	{
		const int vertexCnt = col * row;
		const int cellCnt = (col - 1) * (row - 1);

		graphic::cVertexLayout vtxLayout;
		vtxLayout.Create(
			graphic::eVertexType::POSITION 
			| graphic::eVertexType::NORMAL
			| graphic::eVertexType::COLOR
		);
		m_posOffset = vtxLayout.GetOffset("POSITION");
		m_normOffset = vtxLayout.GetOffset("NORMAL");
		m_colorOffset = vtxLayout.GetOffset("COLOR");
		m_vertexStride = vtxLayout.GetVertexSize();
		m_vtxBuff.Create(renderer, cellCnt * 6, m_vertexStride, D3D11_USAGE_DYNAMIC);
	}

	// I am essentially using this vector as an array with room for 
	// num_particles_width*num_particles_height particles
	m_particles.resize(col*row);

	// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
	for (int x = 0; x < col; x++)
	{
		for (int y = 0; y < row; y++)
		{
			Vector3 pos = Vector3(width * (x / (float)col),
				(height*1.5f) - height * (y / (float)row),
				0);
			// insert particle in column x at y'th row
			m_particles[y*col + x] = cParticle(pos);
		}
	}

	// The cloth is seen as consisting of triangles for four particles in the grid as follows:
	//
	// (x,y)   *--* (x+1,y)
	// 		   | /|
	// 		   |/ |
	// (x,y+1) *--* (x+1,y+1)
	//
	//    (p1) *--* (p2)
	//         | /|
	//         |/ |
	//    (p3) *--* (p4)
	//
	// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
	cParticle *p = &m_particles[0];
	for (int x = 0; x < col; x++)
	{
		for (int y = 0; y < row; y++)
		{
			cParticle *p1 = p + y * m_col + x;
			cParticle *p2 = p + y * m_col + x + 1;
			cParticle *p3 = p + (y + 1) * m_col + x;
			cParticle *p4 = p + (y + 1) * m_col + x + 1;

			if (x < col - 1)
				MakeConstraint(p1, p2);
			if (y < row - 1)
				MakeConstraint(p1, p3);
			if (x < col - 1 && y < row - 1)
				MakeConstraint(p1, p4);
			if (x < col - 1 && y < row - 1)
				MakeConstraint(p2, p3);
		}
	}

	// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
	for (int x = 0; x < col; x++)
	{
		for (int y = 0; y < row; y++)
		{
			cParticle *p1 = p + y * m_col + x;
			cParticle *p2 = p + y * m_col + x + 2;
			cParticle *p3 = p + (y + 2) * m_col + x;
			cParticle *p4 = p + (y + 2) * m_col + x + 2;

			if (x < col - 2)
				MakeConstraint(p1, p2);
			if (y < row - 2)
				MakeConstraint(p1, p3);
			if (x < col - 2 && y < row - 2)
				MakeConstraint(p1, p4);
			if (x < col - 2 && y < row - 2)
				MakeConstraint(p2, p3);
		}
	}

	// making the upper left most three and right most three particles unmovable
	for (int i = 0; i < 3; i++)
	{
		// moving the particle a bit towards the center, to make it hang more natural 
		// - because I like it ;)
		GetParticle(0 + i, 0)->OffsetPos(Vector3(0.5, 0.0, 0.0));
		GetParticle(0 + i, 0)->MakeUnmovable();

		// moving the particle a bit towards the center, to make it hang more natural 
		// - because I like it ;)
		GetParticle(0 + i, 0)->OffsetPos(Vector3(-0.5, 0.0, 0.0));
		GetParticle(col - 1 - i, 0)->MakeUnmovable();
	}

	// create triangle index array
	// The cloth is seen as consisting of triangles for four particles in the grid as follows:
	//
	// (x,y)   *--* (x+1,y)
	// 		   | /|
	// 		   |/ |
	// (x,y+1) *--* (x+1,y+1)
	//
	//    (p1) *--* (p2,p4)
	//         | /|
	//         |/ |
	// (p3,p6) *--* (p4,p5)
	{
		const int cellCnt = (col - 1) * (row - 1);
		m_indices.reserve(cellCnt * 2 * 3);
		for (int x = 0; x < col - 1; x++)
		{
			for (int y = 0; y < row - 1; y++)
			{
				m_indices.push_back(y * col + x); // p1
				m_indices.push_back(y * col + x + 1); // p2
				m_indices.push_back((y + 1) * col + x); // p3
				m_indices.push_back(y * col + x + 1); // p4
				m_indices.push_back((y + 1) * col + x + 1); // p5
				m_indices.push_back((y + 1) * col + x); // p6
			}
		}
	}

	return true;
}


// drawing the cloth as a smooth shaded (and colored according to column) OpenGL triangular mesh
// Called from the display() method
void cCloth::DrawShaded(graphic::cRenderer &renderer)
{
	// reset normals (which where written to last frame)
	{
		cParticle *p = &m_particles[0];
		for (uint i = 0; i < m_particles.size(); ++i)
			p++->m_accumulated_normal = Vector3(0, 0, 0);
	}

	// create smooth per particle normals by adding up
	// all the (hard) triangle normals that each particle is part of
	{
		cParticle *p = &m_particles[0];
		WORD *i = &m_indices[0];
		for (uint k = 0; k < m_indices.size(); k+=3, i+=3)
		{
			cParticle *p1 = p + *(i + 0);
			cParticle *p2 = p + *(i + 1);
			cParticle *p3 = p + *(i + 2);
			Vector3 normal = CalcTriangleNormal(p1, p2, p3).Normal();
			p1->m_accumulated_normal += normal;
			p2->m_accumulated_normal += normal;
			p3->m_accumulated_normal += normal;
		}
	}

	if (BYTE *pb = (BYTE*)m_vtxBuff.Lock(renderer))
	{
		cParticle *p = &m_particles[0];
		WORD *i = &m_indices[0];
		for (uint k = 0; k < m_indices.size(); k += 3, i += 3)
		{
			cParticle *p1 = p + *(i + 0);
			cParticle *p2 = p + *(i + 1);
			cParticle *p3 = p + *(i + 2);
			Vector3 color(0, 0, 0);
			if ((k / (m_row-1) / 6) % 2) // red and white color is interleaved according to which column number
				color = Vector3(0.6f, 0.2f, 0.2f);
			else
				color = Vector3(1.0f, 1.0f, 1.0f);
			DrawTriangle(pb, p1, p2, p3, color);
			pb += (m_vertexStride * 3);
		}
	}
	m_vtxBuff.Unlock(renderer);

	// d3d11 render
	{
		using namespace graphic;
		cShader11 *shader = renderer.m_shaderMgr.FindShader(
			graphic::eVertexType::POSITION
			| graphic::eVertexType::NORMAL
			| graphic::eVertexType::COLOR);
		assert(shader);
		shader->SetTechnique("Unlit");
		shader->Begin();
		shader->BeginPass(renderer, 0);

		renderer.m_cbPerFrame.m_v->mWorld = Matrix44::Identity.GetMatrixXM();
		renderer.m_cbPerFrame.Update(renderer);
		renderer.m_cbLight.Update(renderer, 1);

		cColor color = cColor::WHITE;
		Vector4 ambient = color.GetColor();// *0.3f;
		ambient.w = color.GetColor().w;
		Vector4 diffuse = color.GetColor();// *0.7f;
		diffuse.w = color.GetColor().w;
		renderer.m_cbMaterial.m_v->ambient = XMVectorSet(ambient.x, ambient.y, ambient.z, ambient.w);
		renderer.m_cbMaterial.m_v->diffuse = XMVectorSet(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
		renderer.m_cbMaterial.Update(renderer, 2);

		m_vtxBuff.Bind(renderer);
		renderer.GetDevContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		renderer.GetDevContext()->Draw(m_vtxBuff.GetVertexCount(), 0);
	}
}


// this is an important methods where the time is progressed one time step for the entire cloth.
// This includes calling satisfyConstraint() for every constraint, 
// and calling TimeStep() for all particles
void cCloth::TimeStep(const float deltaSeconds)
{
	// iterate over all constraints several times
	for (int i = 0; i < m_iterationCount; i++)
	{
		cConstraint *p = &m_constraints[0];
		for (uint i=0; i < m_constraints.size(); ++i)
			p++->SatisfyConstraint();
	}

	// calculate the position of each particle at the next time step.
	cParticle *p = &m_particles[0];
	for (uint i=0; i < m_particles.size(); ++i)
		p++->TimeStep(deltaSeconds);
}


// used to add gravity (or any other arbitrary vector) to all particles
void cCloth::AddForce(const Vector3 &direction)
{
	cParticle *p = &m_particles[0];
	for (uint i=0; i < m_particles.size(); ++i)
		p++->AddForce(direction);
}


// used to add wind forces to all particles, is added for each triangle since 
// the final force is proportional to the triangle area as seen from the wind direction
void cCloth::WindForce(const Vector3 &direction)
{
	cParticle *p = &m_particles[0];
	WORD *i = &m_indices[0];
	for (uint k = 0; k < m_indices.size(); k += 3, i += 3)
	{
		cParticle *p1 = p + *(i + 0);
		cParticle *p2 = p + *(i + 1);
		cParticle *p3 = p + *(i + 2);
		AddWindForcesForTriangle(p1, p2, p3, direction);
	}
}


// used to detect and resolve the collision of the cloth with the ball.
// This is based on a very simples scheme where the position of each particle 
// is simply compared to the sphere and corrected.
// This also means that the sphere can "slip through" if the ball is small 
// enough compared to the distance in the grid bewteen particles
void cCloth::BallCollision(const Vector3 &center, const float radius)
{
	cParticle *p = &m_particles[0];
	for (uint i=0; i < m_particles.size(); ++i)
	{
		Vector3 v = p->m_pos - center;
		float l = v.Length();
		if (v.Length() < radius) // if the particle is inside the ball
		{
			// project the particle to the surface of the ball
			p->OffsetPos(v.Normal()*(radius - l));
		}
		++p;
	}
}


void cCloth::DoFrame()
{
}


cParticle* cCloth::GetParticle(int x, int y) 
{ 
	return &m_particles[y*m_col + x];
}


void cCloth::MakeConstraint(cParticle *p1, cParticle *p2) 
{ 
	m_constraints.push_back(cConstraint(p1, p2)); 
}


// A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the
// normal vector of the triangle defined by the position of the particles p1, p2, and p3.
// The magnitude of the normal vector is equal to the area of 
// the parallelogram defined by p1, p2 and p3
Vector3 cCloth::CalcTriangleNormal(cParticle *p1, cParticle *p2, cParticle *p3)
{
	const Vector3 &pos1 = p1->m_pos;
	const Vector3 &pos2 = p2->m_pos;
	const Vector3 &pos3 = p3->m_pos;

	Vector3 v1 = pos2 - pos1;
	Vector3 v2 = pos3 - pos1;

	return v1.CrossProduct(v2);
}


// A private method used by windForce() to calcualte the wind force for a single triangle
// defined by p1,p2,p3
void cCloth::AddWindForcesForTriangle(cParticle *p1, cParticle *p2, cParticle *p3
	, const Vector3 direction)
{
	Vector3 normal = CalcTriangleNormal(p1, p2, p3);
	Vector3 d = normal.Normal();
	Vector3 force = normal * (d.DotProduct(direction));
	p1->AddForce(force);
	p2->AddForce(force);
	p3->AddForce(force);
}


// A private method used by drawShaded(), that draws a single triangle p1,p2,p3 with a color
void cCloth::DrawTriangle(BYTE *p
	, cParticle *p1, cParticle *p2, cParticle *p3, const Vector3 color)
{
	*(Vector3*)(p + m_posOffset) = p1->m_pos;
	*(Vector3*)(p + m_normOffset) = p1->m_accumulated_normal.Normal();
	*(Vector4*)(p + m_colorOffset) = color;
	p += m_vertexStride;

	*(Vector3*)(p + m_posOffset) = p2->m_pos;
	*(Vector3*)(p + m_normOffset) = p2->m_accumulated_normal.Normal();
	*(Vector4*)(p + m_colorOffset) = color;
	p += m_vertexStride;

	*(Vector3*)(p + m_posOffset) = p3->m_pos;
	*(Vector3*)(p + m_normOffset) = p3->m_accumulated_normal.Normal();
	*(Vector4*)(p + m_colorOffset) = color;
	p += m_vertexStride;
}


int cCloth::GetIndex(int x, int y)
{
	return y * m_col + x;
}


void cCloth::Clear()
{
	m_vtxBuff.Clear();
}
