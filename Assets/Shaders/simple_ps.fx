
typedef
struct VSOutput
{
    float4 sv_position : SV_POSITION;
    float4 world_position : WORLDPOS;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    uint instance : SV_InstanceID;

} PSInput;




#define PI 3.1415926535897932384626433832795

float3x3 calcCotangentFrame(float3 N, float3 p, float2 uv)
{
	// 隣接ピクセルの勾配を取得
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float3 dp1_p = cross(N, dp1);
    float3 dp2_p = cross(dp2, N);

	// 連立一次方程式を解いてTangentとBinormalを求める
    float3 T = dp2_p * duv1.x + dp1_p * duv2.x;
    float3 B = dp2_p * duv1.y + dp1_p * duv2.y;

    float inv_max = rsqrt(max(max(dot(T, T), dot(B, B)), 1e-6));
    return float3x3(T * inv_max, B * inv_max, N);
}

//----------------------------------------------------------------------------
//!	法線マッピングを適用します
//!	@param	[in]	N	法線
//!	@param	[in]	p	ワールド座標
//!	@param	[in]	uv	テクスチャ座標
//----------------------------------------------------------------------------
float3 Normalmap(float3 N, float3 p, float2 uv, Texture2D NormalTexture, SamplerState NormalSampler)
{
	// 法線マップテクスチャを読み込み
    float3 texture_normal = NormalTexture.Sample(NormalSampler, uv).rgb;
	// デコード
    texture_normal = texture_normal * 2 - 1;

	// タンジェント空間を変換
    float3x3 TBN = calcCotangentFrame(N, p, uv);
    return normalize(mul(texture_normal, TBN));
}

float3 Specular(float3 albedo, float metallic, float roughness, float NdotH, float LdotH, float NdotL, float3 lightColor)
{
    float reflectance = 0.04;
    float3 specularColor = lerp(float3(reflectance, reflectance, reflectance), albedo, metallic);
    
    	// Cook-Torrance BRDF 近似高速化 (Optimizing PBR SIGGRAPH2015)
    float roughness4 = roughness * roughness * roughness * roughness;
    float denominator = (NdotH * NdotH * (roughness4 - 1.0) + 1.0) * LdotH + 0.00001;
	
    float3 brdf = roughness4 * rcp(4.0 * PI * denominator * denominator * (roughness + 0.5));
	
    return float3(brdf * NdotL * specularColor * lightColor);
    
}


uint MatIdx : register(b0);
cbuffer system_cbuffer : register(b4)
{
    float4x4 world_matrix;
    float4x4 view_matrix;
    float4x4 projection_matrix;
    float3 eye_position;
    float system_time;
}

struct MaterialData
{
    float4 diffuse_color;
    float metallic;
    float roughness;
    uint tex_idx[10];
};



StructuredBuffer<MaterialData> Mat : register(t0);

SamplerState sampler1 : register(s0);
Texture2D Tex[] : register(t5);


float4 main(PSInput input) : SV_TARGET
{

    float4 output_color0 = float4(1, 1, 1, 1);
    MaterialData mat = Mat[MatIdx];
    Texture2D diffuse_texture = Tex[mat.tex_idx[0]];
    Texture2D normal_texture = Tex[mat.tex_idx[1]];
    Texture2D roughness_texture = Tex[mat.tex_idx[2]];
    Texture2D metallic_texture = Tex[mat.tex_idx[3]];
    Texture2D emission_texture = Tex[mat.tex_idx[4]];
    float4 texture_color = diffuse_texture.Sample(sampler1, input.uv);
    output_color0 = input.color;
    texture_color *= mat.diffuse_color;
    
    //αチェック
    if (texture_color.a < 0.5)
    {
    }
    //output_color0.rgb *= input.color * frac(system_time);
    //output_color0.a = 1.0;
    float3 albedo = texture_color.rgb;
    float metallic = metallic_texture.SampleLevel(sampler1, input.uv, 5).r;
    float roughness = roughness_texture.SampleLevel(sampler1, input.uv, 5).r;
    metallic = 0.7;
    roughness = 0.7;
    
    static float3 L = normalize(float3(0.0,1.0,-1.0));
    float3 N = Normalmap(normalize(input.normal), input.world_position.xyz, input.uv, normal_texture, sampler1);
    float3 V = normalize(eye_position - input.world_position.xyz);
    float3 H = normalize(L + V);
    float NdotH = saturate(dot(N, H)) + 0.000001;
    float LdotH = saturate(dot(L, H)) + 0.000001;
    float NdotV = saturate(dot(N, V)) + 0.000001;
    float NdotL = saturate(dot(N, L)) + 0.000001;

    float3 lightColor = float3(1.0, 1.0, 1.0) * 5;
    
    float reflectance = 0.04;
    float3 specularColor = lerp(float3(reflectance, reflectance, reflectance), albedo, metallic);

    
   
    const float Kd = 1.0 / PI;
    float3 diffuse = lightColor * (NdotL * Kd) * albedo;
    float3 specular = Specular(albedo, metallic, roughness, NdotH, LdotH, NdotL, lightColor);
    output_color0.rgb *= diffuse + specular;
    
    float3 ambient = float3(0.1, 0.1, 0.1) * albedo;
    output_color0.rgb += ambient;
    
    return output_color0;

}