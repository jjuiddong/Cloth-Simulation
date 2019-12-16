//--------------------------------------------------------------------------------------
// File: cloth.fx
// cloth simulation compute shader
//
//--------------------------------------------------------------------------------------

cbuffer cb0 : register(b6)
{
	float4 g_deltaSeconds;
	float4 g_force;
}

struct sConstraints
{
	int p1;
	int p2;
	float rest_distance;
};

struct sConstraints2
{
	int p1;
	int p2;
};

// SatisfyConstraint
struct sParticle1
{
	float movable;
	float mass;
	float4 pos;
	float4 oldPos;
	float4 acceleration;
	float4 accumulated_normal;
};


StructuredBuffer<sConstraints> g_constraints : register(t0);
StructuredBuffer<sParticle1> g_particles : register(t1);
StructuredBuffer<sParticle1> g_particles2 : register(t0);
RWStructuredBuffer<sParticle1> g_out1 : register(u0);



[numthreads(1, 1, 1)]
void CS_AddForce(uint3 DTid : SV_DispatchThreadID
	//, uint index : SV_GroupIndex
)
{
	float movable = g_particles2[DTid.x].movable;
	float mass = g_particles2[DTid.x].mass;
	float3 acceleration = g_particles2[DTid.x].acceleration.xyz;

	if (movable > 0)
		acceleration += g_force.xyz / mass;

	g_out1[DTid.x].movable = movable;
	g_out1[DTid.x].mass = mass;
	g_out1[DTid.x].acceleration = float4(acceleration, 0);
	g_out1[DTid.x].pos = g_particles2[DTid.x].pos;
	g_out1[DTid.x].oldPos = g_particles2[DTid.x].oldPos;
	g_out1[DTid.x].accumulated_normal = g_particles2[DTid.x].accumulated_normal;
}



[numthreads(1, 1, 1)]
void CS_SatisfyConstraint(uint3 DTid : SV_DispatchThreadID
	//, uint index : SV_GroupIndex
)
{
	uint i1 = g_constraints[DTid.x].p1;
	uint i2 = g_constraints[DTid.x].p2;
	float rest_distance = g_constraints[DTid.x].rest_distance;

	float movable1 = g_particles[i1].movable;
	float movable2 = g_particles[i2].movable;
	float3 pos1 = g_particles[i1].pos.xyz;
	float3 pos2 = g_particles[i2].pos.xyz;

	float3 p1_to_p2 = pos2 - pos1;
	float current_distance = length(p1_to_p2);
	float3 correctionVector = p1_to_p2 * max(0.f, min(1.f, (1.f - (rest_distance / current_distance))));
	float3 correctionVectorHalf = correctionVector * 0.5f;

	float3 cpos1 = (movable1 > 0.f) ? (pos1 + correctionVectorHalf) : pos1;
	float3 cpos2 = (movable2 > 0.f) ? (pos2 - correctionVectorHalf) : pos2;
	//float3 cpos1 = pos1;
	//float3 cpos2 = pos2;

	//g_out1[i1].movable = movable1;
	//g_out1[i2].movable = movable2;
	g_out1[i1].pos = float4(cpos1, 0);
	g_out1[i2].pos = float4(cpos2, 0);

	//g_out1[i1] = g_particles[i1];
	//g_out1[i2] = g_particles[i2];
	//g_out1[i1].pos = float4(cpos1, 0);
	//g_out1[i2].pos = float4(cpos2, 0);

	//g_out1[DTid.x].pos = float4(pos, 0);
	//g_out1[DTid.x].oldPos = float4(oldPos, 0);
	//g_out1[DTid.x].acceleration = float4(acceleration, 0);
	//g_out1[DTid.x].accumulated_normal = float4(accumulated_normal, 0);
}



[numthreads(1, 1, 1)]
void CS_TimeStep(uint3 DTid : SV_DispatchThreadID
	//, uint index : SV_GroupIndex
)
{
	float movable = g_particles[DTid.x].movable;
	float mass = g_particles[DTid.x].mass;
	float3 pos = g_particles[DTid.x].pos.xyz;
	float3 oldPos = g_particles[DTid.x].oldPos.xyz;
	float3 acceleration = g_particles[DTid.x].acceleration.xyz;
	float3 accumulated_normal = g_particles[DTid.x].accumulated_normal.xyz;

	const float damping = 0.01f;

	if (movable > 0)
	{
		float3 temp = pos;
		pos = pos + (pos - oldPos) * (1.0f - damping)
			+ acceleration * 0.001f * 0.001f
			;
		oldPos = temp;
		acceleration = float3(0, 0, 0);
	}

	g_out1[DTid.x].movable = movable;
	g_out1[DTid.x].mass = mass;
	g_out1[DTid.x].pos = float4(pos, 0);
	g_out1[DTid.x].oldPos = float4(oldPos, 0);
	g_out1[DTid.x].acceleration = float4(acceleration, 0);
	g_out1[DTid.x].accumulated_normal = float4(accumulated_normal, 0);
}


technique11 AddForce
{
	pass P0
	{
		SetVertexShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, CS_AddForce()));
		SetGeometryShader(NULL);
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetPixelShader(NULL);
	}
}



technique11 SatisfyConstraint
{
	pass P0
	{
		SetVertexShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, CS_SatisfyConstraint()));
		SetGeometryShader(NULL);
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetPixelShader(NULL);
	}
}


technique11 TimeStep
{
	pass P0
	{
		SetVertexShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, CS_TimeStep()));
		SetGeometryShader(NULL);
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetPixelShader(NULL);
	}
}
