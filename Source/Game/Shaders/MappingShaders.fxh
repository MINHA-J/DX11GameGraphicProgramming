//--------------------------------------------------------------------------------------
// File: Shadersasd.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (1)
//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
TextureCube EnvironmentMap : register(t1);
SamplerState TextureSampler : register(s0);


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement
  Summary:  Constant buffer used for view transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize
  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame
  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
    bool HasNormalMap;
};

struct stPointLight
{
    float4 Position;
    float4 Color;
    float4 AttenuationDistance;
};

cbuffer cbLights : register(b3)
{
    //float4 LightPositions[NUM_LIGHTS];
    //float4 LightColors[NUM_LIGHTS];
    //matrix LightViews[NUM_LIGHTS];
    //matrix LightProjections[NUM_LIGHTS];
    stPointLight PointLight[NUM_LIGHTS];
};


//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
    float4 Position : POSITION;
    float3 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_INPUT
{
    float4 Position : SV_POSITION; // interpolated vertex position (system value) 
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL; //?
    float3 WorldPosition : WORLDPOS;
    float3 ReflectionVector : REFLECTION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VSEnvironmentMap(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.TexCoord = input.TexCoord;

    float3 worldPosition = mul(input.Position, World).xyz;
    float3 incident = normalize(worldPosition - CameraPosition.xyz);
    float3 normal = normalize(mul(float4(input.Normal, 0), World).xyz);

	// Reflection Vector for cube map: R = I - 2*N * (I.N)
    output.ReflectionVector = reflect(incident, normal);
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSEnvironmentMap(PS_INPUT input) : SV_Target
{
    // Compute the reflection vector
    
    // Use this reflection vector as the texture coordinate to sample the skybox
    float4 diffuseColor = diffuseTexture.Sample(TextureSampler, input.TexCoord);
    float3 environment = EnvironmentMap.Sample(TextureSampler, input.ReflectionVector).rgb;
    // ambient
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0u; i < NUM_LIGHTS; ++i)
    {
        ambient += float4(float3(0.1f, 0.1f, 0.1f) * PointLight[i].Color.xyz, 1.0f);
    }
    
    // diffuse
    float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    for (uint j = 0u; j < NUM_LIGHTS; ++j)
    {
        lightDirection = normalize(PointLight[j].Position.xyz - input.WorldPosition);
        diffuse += saturate(dot(input.Normal, lightDirection)) * PointLight[j].Color;
    }

    // specular
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 reflectDirection = float3(0.0f, 0.0f, 0.0f);
    float shiness = 20.0f;
    for (uint k = 0; k < NUM_LIGHTS; ++k)
    {
        lightDirection = normalize(PointLight[k].Position.xyz - input.WorldPosition);
        reflectDirection = reflect(-lightDirection, input.Normal);
        specular += pow(saturate(dot(reflectDirection, viewDirection)), shiness) * PointLight[k].Color;
    }

    return float4(saturate(ambient + diffuse + specular + environment * 0.5f), diffuseColor.a);;
}