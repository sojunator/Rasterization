cbuffer ConstantBuffer : register(b0)
{
	float3 Offset;


};
cbuffer ConstantBuffer : register(b1)
{
	float3 Color;

};



struct VS_IN
{
	float3 Pos : POSITION;
	float3 Color : COLOR;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float3 Color : COLOR;
};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.Pos = float4(input.Pos, 1.0f);
	output.Pos.x += Offset.x;
	output.Pos.y += Offset.y;
	output.Pos.xy *= Offset.z;

	output.Color = input.Color;
	output.Color *= Color;
	return output;
}