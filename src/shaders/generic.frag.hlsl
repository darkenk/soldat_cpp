// Fragment shader converted from GLSL to HLSL
// Preprocessor definition for dithering control
#define DITHERING 0

// Texture and sampler declarations
Texture2D tex : register(t0, space2);
SamplerState texSampler : register(s0, space2);
#if DITHERING
Texture2D dither : register(t1, space2);
SamplerState ditherSampler : register(s1, space2);
#endif // DITHERING

// Input structure matching vertex shader output
struct PS_INPUT
{
  float4 Position : SV_Position;  // gl_FragCoord equivalent
  float2 UV : TEXCOORD0;
  float4 Color : COLOR0;
};

// Pixel shader main function
float4 main(PS_INPUT input) : SV_Target0
{
  // Sample texture and multiply with input color (same as original)
  float4 outputColor = tex.Sample(texSampler, input.UV) * input.Color;
  
  #if DITHERING
  // Apply dithering pattern using screen coordinates
  outputColor.rgb += dither.Sample(ditherSampler, input.Position.xy / 8.0).aaa / 32.0 - 1.0/128.0;
  #endif

  return outputColor;
}