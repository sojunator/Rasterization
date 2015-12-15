cbuffer ConstantBuffer
{
	matrix finalMatrix;
	matrix world;    // the rotation matrix
	float4 lightvec;      // the light's vector
	float4 lightcol;      // the light's color
	float4 ambientcol;    // the ambient light's color
};

struct GSInput
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 normal : NORMAL;
};

[maxvertexcount(6)]
void GS_main(triangle GSInput input[3], inout TriangleStream< GSOutput > output)
{
	// Define our edges
	float3 edge1 = input[0].Pos.xyz;
	float3 edge2 = input[1].Pos.xyz;
	float3 edge3 = input[2].Pos.xyz;

	// Create vectors for the face
	float3 vec1 = edge2 - edge1;
	float3 vec2 = edge3 - edge1;

	// Calculate the normal for the face and normalize it
	
	float4 normal = float4(cross(vec1, vec2), 1.0f);
	normal = normalize(normal);

	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = mul(input[i].Pos, finalMatrix);
		element.Tex = input[i].Tex;
		element.normal = normal;
		output.Append(element);
	}
	output.RestartStrip();
	// Create 3 new vertices with a new position, using the normal as offset
	for (uint j = 0; j < 3; j++)
	{
		GSOutput element_1;
		element_1.pos.x = input[j].Pos.x + normal.x;
		element_1.pos.y = input[j].Pos.y + normal.y;
		element_1.pos.z = input[j].Pos.z + normal.z;
		element_1.pos[3] = 1.0f; // fill every position in the element;
		element_1.pos = mul(element_1.pos, finalMatrix);
		element_1.Tex = input[j].Tex;
		element_1.normal = normal;
		output.Append(element_1);
	}

}	