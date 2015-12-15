cbuffer ConstantBuffer
{
	matrix finalMatrix;
	float4x4 world;    // the rotation matrix
	float4 lightvec;      // the light's vector
	float4 lightcol;      // the light's color
	float4 ambientcol;    // the ambient light's color
};

Texture2D txDiffuse : register(t0);
SamplerState sampAni;
struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
float4 PS_main(VS_OUT input) : SV_Target
{
	float3 s = txDiffuse.Sample(sampAni, input.Tex).xyz;
	return float4(s, 1.0);
}