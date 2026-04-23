struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    uint instance_id : SV_InstanceID;
};

struct VSOutput
{
    float4 sv_position : SV_POSITION;
    float4 world_position : WORLDPOS;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    uint instance : SV_InstanceID;
};
#define PI 3.1415926535897932384626433832795
#define TWO_PI (2.0 * PI)
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)
cbuffer system_cbuffer : register(b4)
{
    float4x4 world_matrix;
    float4x4 view_matrix;
    float4x4 projection_matrix;
    float3 eye_position;
    float system_time;
}


StructuredBuffer<float4x4> ObjWorld : register(t1);

VSOutput main(VSInput input)
{

    VSOutput output;

    float3 world_position = mul(ObjWorld[input.instance_id], float4(input.pos, 1.0)).xyz;
    float3 view_position = mul(view_matrix, float4(world_position, 1.0)).xyz;
    float4 screen_position = mul(projection_matrix, float4(view_position, 1.0));
    output.world_position = float4(world_position, 1.0);
    output.sv_position = screen_position;
    output.color = input.color;
    output.uv = input.uv;
    output.normal = mul(ObjWorld[input.instance_id], float4(input.normal, 0.0)).xyz;
    output.instance = input.instance_id;
    return output;

}