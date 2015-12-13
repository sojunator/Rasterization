cbuffer ConstantBuffer
{
	matrix finalMatrix;
};

struct GSInput
{
	float4 Pos : SV_POSITION;
	float3 Color : COLOR;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 Color : COLOR;
};

[maxvertexcount(6)]
void GS_main(triangle GSInput input[3], inout TriangleStream< GSOutput > output)
{
	// Define our edges
	float3 edge1 = input[0].Pos.xzy;
	float3 edge2 = input[1].Pos.xzy;
	float3 edge3 = input[2].Pos.xzy;

	// Create vectors for the face
	float3 vec1 = edge1 - edge2;
	float3 vec2 = edge1 - edge3;

	// Calculate the normal for the face and normalize it
	
	float4 normal = float4(cross(vec1, vec2), 1.0f);
	float4 normalizedNormal = normalize(normal);
    normalizedNormal[3] = 1.0f;

	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = mul(input[i].Pos, finalMatrix);
		element.Color = input[i].Color;
		output.Append(element);
	}
	output.RestartStrip();
	// Create 3 new vertices with a new position, using the normal as offset
	for (uint j = 0; j < 3; j++)
	{
		GSOutput element_1;
		element_1.pos.x = input[j].Pos.x + normalizedNormal.x;
		element_1.pos.y = input[j].Pos.y + normalizedNormal.y;
		element_1.pos.z = input[j].Pos.z + normalizedNormal.z;
		element_1.pos[3] = 1.0f; // fill every position in the element;

		element_1.pos = mul(element_1.pos, finalMatrix);

		element_1.Color = input[j].Color;
		output.Append(element_1);
	}

}	