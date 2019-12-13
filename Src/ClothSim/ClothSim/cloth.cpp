
#include "stdafx.h"
#include "cloth.h"


cCloth::cCloth()
{
}

cCloth::~cCloth()
{
	Clear();
}


// This is a important constructor for the entire system of particles and constraints
bool cCloth::Init(graphic::cRenderer &renderer
	, float width, float height, int num_particles_width0, int num_particles_height0)
{

	// create vertex, index buffer
	{
		const int vertexCnt = num_particles_width0 * num_particles_height0;
		const int cellCnt = (num_particles_width0 - 1) * (num_particles_height0 - 1);

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

		vector<WORD> indices(cellCnt * 2 * 3);
		int idx = 0;
		for (int x = 0; x < num_particles_width0 - 1; x++)
		{
			for (int y = 0; y < num_particles_height0 - 1; y++)
			{
				// triangle1
				indices[idx] = idx++;
				indices[idx] = idx++;
				indices[idx] = idx++;
				// triangle2
				indices[idx] = idx++;
				indices[idx] = idx++;
				indices[idx] = idx++;
			}
		}
		m_idxBuff.Create(renderer, cellCnt*2, (BYTE*)&indices[0]);
	}

	m_num_particles_width = num_particles_width0;
	m_num_particles_height = num_particles_height0;

	// I am essentially using this vector as an array with room for 
	// num_particles_width*num_particles_height particles
	m_particles.resize(num_particles_width0*num_particles_height0);

	// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
	for (int x = 0; x < num_particles_width0; x++)
	{
		for (int y = 0; y < num_particles_height0; y++)
		{
			Vector3 pos = Vector3(width * (x / (float)num_particles_width0),
				(height*1.5f) - height * (y / (float)num_particles_height0),
				0);
			// insert particle in column x at y'th row
			m_particles[y*num_particles_width0 + x] = cParticle(pos);
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
	for (int x = 0; x < num_particles_width0; x++)
	{
		for (int y = 0; y < num_particles_height0; y++)
		{
			cParticle *p1 = p + y * m_num_particles_width + x;
			cParticle *p2 = p + y * m_num_particles_width + x + 1;
			cParticle *p3 = p + (y + 1) * m_num_particles_width + x;
			cParticle *p4 = p + (y + 1) * m_num_particles_width + x + 1;

			if (x < num_particles_width0 - 1)
				makeConstraint(p1, p2);
			if (y < num_particles_height0 - 1)
				makeConstraint(p1, p3);
			if (x < num_particles_width0 - 1 && y < num_particles_height0 - 1)
				makeConstraint(p1, p4);
			if (x < num_particles_width0 - 1 && y < num_particles_height0 - 1)
				makeConstraint(p2, p3);
		}
	}

	// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
	for (int x = 0; x < num_particles_width0; x++)
	{
		for (int y = 0; y < num_particles_height0; y++)
		{
			cParticle *p1 = p + y * m_num_particles_width + x;
			cParticle *p2 = p + y * m_num_particles_width + x + 2;
			cParticle *p3 = p + (y + 2) * m_num_particles_width + x;
			cParticle *p4 = p + (y + 2) * m_num_particles_width + x + 2;

			if (x < num_particles_width0 - 2)
				makeConstraint(p1, p2);
			if (y < num_particles_height0 - 2)
				makeConstraint(p1, p3);
			if (x < num_particles_width0 - 2 && y < num_particles_height0 - 2)
				makeConstraint(p1, p4);
			if (x < num_particles_width0 - 2 && y < num_particles_height0 - 2)
				makeConstraint(p2, p3);
		}
	}

	// making the upper left most three and right most three particles unmovable
	for (int i = 0; i < 3; i++)
	{
		// moving the particle a bit towards the center, to make it hang more natural 
		// - because I like it ;)
		getParticle(0 + i, 0)->offsetPos(Vector3(0.5, 0.0, 0.0));
		getParticle(0 + i, 0)->makeUnmovable();

		// moving the particle a bit towards the center, to make it hang more natural 
		// - because I like it ;)
		getParticle(0 + i, 0)->offsetPos(Vector3(-0.5, 0.0, 0.0));
		getParticle(num_particles_width0 - 1 - i, 0)->makeUnmovable();
	}

	return true;
}


// drawing the cloth as a smooth shaded (and colored according to column) OpenGL triangular mesh
// Called from the display() method
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
//
void cCloth::drawShaded(graphic::cRenderer &renderer)
{
	// reset normals (which where written to last frame)
	{
		cParticle *p = &m_particles[0];
		for (uint i = 0; i < m_particles.size(); ++i)
			p++->accumulated_normal = Vector3(0, 0, 0);
	}

	// create smooth per particle normals by adding up
	// all the (hard) triangle normals that each particle is part of
	{
		cParticle *p = &m_particles[0];
		for (int x = 0; x < m_num_particles_width - 1; x++)
		{
			for (int y = 0; y < m_num_particles_height - 1; y++)
			{
				cParticle *p1 = p + y * m_num_particles_width + x;
				cParticle *p2 = p + y * m_num_particles_width + x + 1;
				cParticle *p3 = p + (y + 1) * m_num_particles_width + x;
				Vector3 normal = calcTriangleNormal(p1, p2, p3).Normal();
				p1->accumulated_normal += normal;
				p2->accumulated_normal += normal;
				p3->accumulated_normal += normal;

				cParticle *p4 = p + y * m_num_particles_width + x + 1;
				cParticle *p5 = p + (y + 1) * m_num_particles_width + x + 1;
				cParticle *p6 = p + (y + 1) * m_num_particles_width + x;
				Vector3 normal2 = calcTriangleNormal(p4, p5, p6).Normal();
				p4->accumulated_normal += normal2;
				p5->accumulated_normal += normal2;
				p6->accumulated_normal += normal2;
			}
		}
	}

	if (BYTE *pb = (BYTE*)m_vtxBuff.Lock(renderer))
	{
		cParticle *p = &m_particles[0];
		for (int x = 0; x < m_num_particles_width - 1; x++)
		{
			for (int y = 0; y < m_num_particles_height - 1; y++)
			{
				Vector3 color(0, 0, 0);
				if (x % 2) // red and white color is interleaved according to which column number
					color = Vector3(0.6f, 0.2f, 0.2f);
				else
					color = Vector3(1.0f, 1.0f, 1.0f);

				cParticle *p1 = p + y * m_num_particles_width + x;
				cParticle *p2 = p + y * m_num_particles_width + x + 1;
				cParticle *p3 = p + (y + 1) * m_num_particles_width + x;
				cParticle *p4 = p + y * m_num_particles_width + x + 1;
				cParticle *p5 = p + (y + 1) * m_num_particles_width + x + 1;
				cParticle *p6 = p + (y + 1) * m_num_particles_width + x;

				drawTriangle(pb, p1, p2, p3, color);
				pb += (m_vertexStride * 3);
				drawTriangle(pb, p4, p5, p6, color);
				pb += (m_vertexStride * 3);
			}
		}
	}
	m_vtxBuff.Unlock(renderer);


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
		m_idxBuff.Bind(renderer);
		renderer.GetDevContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		renderer.GetDevContext()->DrawIndexed(m_idxBuff.GetFaceCount() * 3, 0, 0);
	}
}


// this is an important methods where the time is progressed one time step for the entire cloth.
// This includes calling satisfyConstraint() for every constraint, 
// and calling timeStep() for all particles
void cCloth::timeStep()
{
	for (int i = 0; i < CONSTRAINT_ITERATIONS; i++) // iterate over all constraints several times
	{
		cConstraint *p = &m_constraints[0];
		for (uint i=0; i < m_constraints.size(); ++i)
			p++->satisfyConstraint(); // satisfy constraint.
	}

	cParticle *p = &m_particles[0];
	for (uint i=0; i < m_particles.size(); ++i)
		p++->timeStep(); // calculate the position of each particle at the next time step.
}


// used to add gravity (or any other arbitrary vector) to all particles
void cCloth::addForce(const Vector3 &direction)
{
	cParticle *p = &m_particles[0];
	for (uint i=0; i < m_particles.size(); ++i)
		p++->addForce(direction);
}


// used to add wind forces to all particles, is added for each triangle since 
// the final force is proportional to the triangle area as seen from the wind direction
// (x,y)   *--* (x+1,y)
// 		   | /|
// 		   |/ |
// (x,y+1) *--* (x+1,y+1)
//
//    (p1) *--* (p2,p4)
//         | /|
//         |/ |
// (p3,p6) *--* (p4,p5)
void cCloth::windForce(const Vector3 &direction)
{
	cParticle *p = &m_particles[0];
	for (int x = 0; x < m_num_particles_width - 1; x++)
	{
		for (int y = 0; y < m_num_particles_height - 1; y++)
		{
			cParticle *p1 = p + y * m_num_particles_width + x;
			cParticle *p2 = p + y * m_num_particles_width + x + 1;
			cParticle *p3 = p + (y + 1) * m_num_particles_width + x;
			cParticle *p4 = p + y * m_num_particles_width + x + 1;
			cParticle *p5 = p + (y + 1) * m_num_particles_width + x + 1;
			cParticle *p6 = p + (y + 1) * m_num_particles_width + x;

			addWindForcesForTriangle(p1, p2, p3, direction);
			addWindForcesForTriangle(p4, p5, p6, direction);
		}
	}
}


// used to detect and resolve the collision of the cloth with the ball.
// This is based on a very simples scheme where the position of each particle 
// is simply compared to the sphere and corrected.
// This also means that the sphere can "slip through" if the ball is small 
// enough compared to the distance in the grid bewteen particles
void cCloth::ballCollision(const Vector3 &center, const float radius)
{
	cParticle *p = &m_particles[0];
	for (uint i=0; i < m_particles.size(); ++i)
	{
		Vector3 v = p->pos - center;
		float l = v.Length();
		if (v.Length() < radius) // if the particle is inside the ball
		{
			// project the particle to the surface of the ball
			p->offsetPos(v.Normal()*(radius - l));
		}
		++p;
	}
}


void cCloth::doFrame()
{
}


cParticle* cCloth::getParticle(int x, int y) 
{ 
	return &m_particles[y*m_num_particles_width + x];
}


void cCloth::makeConstraint(cParticle *p1, cParticle *p2) 
{ 
	m_constraints.push_back(cConstraint(p1, p2)); 
}


// A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the
// normal vector of the triangle defined by the position of the particles p1, p2, and p3.
// The magnitude of the normal vector is equal to the area of 
// the parallelogram defined by p1, p2 and p3
Vector3 cCloth::calcTriangleNormal(cParticle *p1, cParticle *p2, cParticle *p3)
{
	const Vector3 &pos1 = p1->pos;
	const Vector3 &pos2 = p2->pos;
	const Vector3 &pos3 = p3->pos;

	Vector3 v1 = pos2 - pos1;
	Vector3 v2 = pos3 - pos1;

	return v1.CrossProduct(v2);
}


// A private method used by windForce() to calcualte the wind force for a single triangle
// defined by p1,p2,p3
void cCloth::addWindForcesForTriangle(cParticle *p1, cParticle *p2, cParticle *p3
	, const Vector3 direction)
{
	Vector3 normal = calcTriangleNormal(p1, p2, p3);
	Vector3 d = normal.Normal();
	Vector3 force = normal * (d.DotProduct(direction));
	p1->addForce(force);
	p2->addForce(force);
	p3->addForce(force);
}


// A private method used by drawShaded(), that draws a single triangle p1,p2,p3 with a color
void cCloth::drawTriangle(BYTE *p
	, cParticle *p1, cParticle *p2, cParticle *p3, const Vector3 color)
{
	*(Vector3*)(p + m_posOffset) = p1->pos;
	*(Vector3*)(p + m_normOffset) = p1->accumulated_normal.Normal();
	*(Vector4*)(p + m_colorOffset) = color;
	p += m_vertexStride;

	*(Vector3*)(p + m_posOffset) = p2->pos;
	*(Vector3*)(p + m_normOffset) = p2->accumulated_normal.Normal();
	*(Vector4*)(p + m_colorOffset) = color;
	p += m_vertexStride;

	*(Vector3*)(p + m_posOffset) = p3->pos;
	*(Vector3*)(p + m_normOffset) = p3->accumulated_normal.Normal();
	*(Vector4*)(p + m_colorOffset) = color;
	p += m_vertexStride;
}


int cCloth::getIndex(int x, int y)
{
	return y * m_num_particles_width + x;
}


void cCloth::Clear()
{
	m_vtxBuff.Clear();
	m_idxBuff.Clear();
}
