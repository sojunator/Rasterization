Texture2D txDiffuse : register(t0);
SamplerState sampAni;
struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 normal : NORMAL;
	float4 worldPos : POSITION;
};

float3 ads(VS_OUT input, float3 kd)
{
 	float3 n = normalize(input.normal.xyz);
	float3 s = normalize(float3(0.0f, 0.0f, -4.0f) - input.worldPos.xyz);
	float3 v = normalize(float3(-input.worldPos.xyz));
	float3 r = reflect(-s, n);
	float3 diffuseLight = kd * max(dot(s, n), 0.0);
	float shinyPower = 2000.0f;
	float3 Ka = float3(0.2f, 0.2f, 0.2f);
	return float3(1.0f,1.0f,1.0f) * (diffuseLight);
}

float4 PS_main(VS_OUT input) : SV_Target
{
	float3 s = txDiffuse.Sample(sampAni, input.Tex).xyz;
	return float4(ads(input, s), 1.0f) * float4(s, 1.0);
}