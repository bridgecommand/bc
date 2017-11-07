float4x4 WorldViewProjection;
//float lightLevel;

float4 vs_main(float4 inputPosition : POSITION) : POSITION
{
    return mul(inputPosition, WorldViewProjection);
}

float4 ps_main(in float lightLevel) : COLOR0
{
	return float4(1, lightLevel, 1, 1);
}