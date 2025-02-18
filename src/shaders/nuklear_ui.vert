// Constant buffer equivalent to the GLSL UBO
cbuffer TransformBuffer : register(b0, space1)  // Set 1, binding 0
{
  float2 uScale;
  float2 uTranslate;
};

// Input structure
struct VS_INPUT
{
  float2 aPos : POSITION;
  float2 aUV : TEXCOORD0;
  float4 aColor : COLOR0;
};

// Output structure
struct VS_OUTPUT
{
  float4 Position : SV_Position;  // gl_Position equivalent
  float4 Color : COLOR0;
  float2 UV : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
  VS_OUTPUT output;
  
  output.Color = input.aColor;
  output.UV = input.aUV;
  output.Position = float4(input.aPos * uScale + uTranslate, 0, 1);
  output.Position.y *= -1.0f;
  
  return output;
}