// Texture and sampler binding
Texture2D tex : register(t0, space2);    // Set 2, binding 0
SamplerState texSampler : register(s0, space2);

// Input structure matching vertex shader output
struct PS_INPUT
{
  float4 Position : SV_Position;  // Required but unused
  float4 Color : COLOR0;
  float2 UV : TEXCOORD0;
};

// Pixel shader main function
float4 main(PS_INPUT input) : SV_Target0
{
  // Sample texture and multiply with input color
  return input.Color; // * tex.Sample(texSampler, input.UV);
}