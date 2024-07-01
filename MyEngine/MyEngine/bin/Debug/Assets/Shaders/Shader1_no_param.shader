{
  "name": "TestShader",
  "Properties": ,
  "vsCode": "#include\"BasicShaderHeader.hlsli\"

Output BasicVS( 
	float4 pos : POSITION, 
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint weight : WEIGHT
)
{
	Output output;

	float w = weight / 100.0f;
	matrix bm = bones[boneno[0]] * w + bones[boneno[1]] * (1 - w);

	pos = mul(bm, pos);
	pos = mul(world, pos);

	output.svpos = mul(mul(proj, view), pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - eye);

	return output;
}",
  "psCode": "#include\"BasicShaderHeader.hlsli\"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float3 lightColor = float3(1, 1, 1);

	float diffuseB = saturate(dot(-light, input.normal)) ;
	float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	float4 texColor = tex.Sample(smp,input.uv);

	return max(toonDif
		* diffuse
		* texColor
		* sph.Sample(smp, sphereMapUV)
		+ spa.Sample(smp, sphereMapUV)
		+ float4(specularB * specular.rgb,1)
	    , float4(texColor * ambient,1));
}",
  "vsEntrance": "BasicVS",
  "psEntrance": "BasicPS",
  "isBoneModel": true
}