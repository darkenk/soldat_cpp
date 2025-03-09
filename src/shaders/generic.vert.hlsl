// Constant buffer for transformation matrix
cbuffer TransformBuffer : register(b0, space1)
{
  float3x3 mvp;
};

// Input structure
struct VS_INPUT
{
  float2 position : POSITION;
  float2 texcoords : TEXCOORD0;
  float4 color : COLOR0;
};

// Output structure
struct VS_OUTPUT
{
  float4 position : SV_Position;
  float2 texcoords : TEXCOORD0;
  float4 color : COLOR0;
};

// Vertex shader main function
VS_OUTPUT main(VS_INPUT input)
{
  VS_OUTPUT output;
  
  // Apply alpha premultiplication, same as original
  output.color = float4(input.color.rgb * input.color.a, input.color.a);
  output.texcoords = input.texcoords;
  
  // Transform position with 3x3 matrix (handling .xyw components)
  //float3 posXYW = float3(input.position.x, input.position.y, input.position.w);
  float3 posXYW = float3(input.position.x, input.position.y, 1.0f);
  float3 transformedXYW = mul(mvp, posXYW);
  
  // Set output position with z=0
  output.position = float4(transformedXYW.xy, 0.5f, transformedXYW.z);
  
  return output;
}