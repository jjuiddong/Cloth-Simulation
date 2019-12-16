
cbuffer CB : register(b0)
{
	unsigned int g_iCount;
};

StructuredBuffer<float> Input : register(t0);
RWStructuredBuffer<float> Result : register(u0);

//groupshared float shared_data[128];
float shared_data[128];

[numthreads(128, 1, 1)]
void CS(uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{

	GroupMemoryBarrierWithGroupSync();
	Result[Gid.x] = shared_data[0];
}



technique11 Compute
{
	pass P0
	{
		SetVertexShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, CS()));
		SetGeometryShader(NULL);
		SetHullShader(NULL);
		SetDomainShader(NULL);
		SetPixelShader(NULL);
	}
}
