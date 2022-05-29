//--------------------------------------------------------------------------------------
// File: SkinningShaders.fx
//
// Copyright (c) Microsoft Corporation.
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static const unsigned int MAX_NUM_BONES = 256u;
/*--------------------------------------------------------------------
  TODO: Declare a diffuse texture and a sampler state (remove the comment)
--------------------------------------------------------------------*/
Texture2D aTextures[2] : register(t0);
SamplerState aSamplers[2] : register(s0);


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbChangeOnCameraMovement definition (remove the comment)
--------------------------------------------------------------------*/
cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbChangeOnResize definition (remove the comment)
--------------------------------------------------------------------*/
cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbChangesEveryFrame definition (remove the comment)
--------------------------------------------------------------------*/
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
    bool HasNormalMap;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbLights definition (remove the comment)
--------------------------------------------------------------------*/
cbuffer cbLights : register(b3)
{
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbSkinning

  Summary:  Constant buffer used for skinning
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbSkinning definition (remove the comment)
--------------------------------------------------------------------*/
cbuffer cbSkinning : register(b4)
{
    matrix BoneTransforms[MAX_NUM_BONES];
};


//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: VS_INPUT definition (remove the comment)
--------------------------------------------------------------------*/
struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    uint4 BoneIndices : BONEINDICES;
    float4 BoneWeights : BONEWEIGHTS;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: PS_PHONG_INPUT definition (remove the comment)
--------------------------------------------------------------------*/
struct PS_PHONG_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};



//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Vertex Shader function VSPhong definition (remove the comment)
--------------------------------------------------------------------*/
PS_PHONG_INPUT VSPhong(VS_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT) 0;
    
    // Calculate skin transform matrix with weighted sum of bone transforms
    
    // Apply skin transformation on vertex in model space
    matrix skinTransform = (matrix) 0;
    skinTransform += BoneTransforms[input.BoneIndices.x] * input.BoneWeights.x;
    skinTransform += BoneTransforms[input.BoneIndices.y] * input.BoneWeights.y;
    skinTransform += BoneTransforms[input.BoneIndices.z] * input.BoneWeights.z;
    skinTransform += BoneTransforms[input.BoneIndices.w] * input.BoneWeights.w;
    
    // Transform vertex to projection space using skin / world / view / projection matrix
    output.Position = mul(input.Position, skinTransform);
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    // Transform normal to world space using skin / world matrix
    output.Normal = normalize(mul(float4(input.Normal, 0), skinTransform).xyz);
    output.Normal = normalize(mul(float4(output.Normal, 0), World).xyz);
    // output.Normal = normalize(mul(float4(input.Normal, 1), World).xyz);

    if (HasNormalMap)
    {
        // Transform tangent and bitangent vector to world space
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }
    
    // position of the vertex in world space
    output.WorldPosition = mul(input.Position, World);

    // texture coordinate
    output.TexCoord = input.TexCoord;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Pixel Shader function PSPhong definition (remove the comment)
--------------------------------------------------------------------*/
float4 PSPhong(PS_PHONG_INPUT input) : SV_TARGET
{
    
    float3 normal = normalize(input.Normal);
    
    if (HasNormalMap)
    {
        // Sample the pixel in the normal map.
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);
    
        // Expand the range of the normal value from (0, +1) to (-1, +1).
        bumpMap = (bumpMap * 2.0f) - 1.0f;
    
        // Calculate the normal from the data in the normal map.
        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);
    
        // Normalize the resulting bump normal and replace existing normal
        normal = normalize(bumpNormal);
    }
    
    // Implement texturing and phong shading
    float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
    float3 viewDirection = float3(0.0f, 0.0f, 0.0f);
    float3 reflectDirection = float3(0.0f, 0.0f, 0.0f);
    float4 TextureColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float lightIntensity = 0.0f;

    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
    TextureColor = aTextures[0].Sample(aSamplers[0], input.TexCoord);
    viewDirection = normalize(CameraPosition.xyz - input.WorldPosition);

    for (uint i = 0; i < NUM_LIGHTS; i++)
    {
        lightDirection = normalize(LightPositions[i].xyz - input.WorldPosition);
        reflectDirection = normalize(reflect(-lightDirection, input.Normal));

        ambient += float3(0.1f, 0.f, 0.1f) * LightColors[i].xyz;
        diffuse += saturate(max(dot(normalize(normal), lightDirection), 0) * LightColors[i].xyz);
        specular += saturate(pow(max(dot(reflectDirection, viewDirection), 0), 20.0f)) * LightColors[i];
    }
    diffuse = saturate(diffuse);
    specular = saturate(specular);
    
    // Calculate the amount of light on this pixel based on the bump map normal value.
    lightIntensity = saturate(dot(normal, lightDirection));
    
    // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
    diffuse = saturate(diffuse * lightIntensity);
    
    return TextureColor * float4(ambient + diffuse + specular, 1.0f);
}