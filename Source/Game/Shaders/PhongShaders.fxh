//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (1)
#define NEAR_PLANE (0.01f)
#define FAR_PLANE (1200.0f)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Declare a diffuse texture and a sampler state (remove the comment)
--------------------------------------------------------------------*/
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D shadowMapTexture : register(t2);

SamplerState diffuseSamplers : register(s0);
SamplerState normalSamplers : register(s1);
SamplerState shadowMapSampler : register(s2);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbChangeOnCameraMovement definition (remove the comment)
--------------------------------------------------------------------*/
cbuffer cbChangeOnCameraMovement : register( b0 )
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
    //matrix LightViews[NUM_LIGHTS];
    //matrix LightProjections[NUM_LIGHTS];
    float4 AttenuationDistance[NUM_LIGHTS];
};


//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: VS_PHONG_INPUT definition (remove the comment)
--------------------------------------------------------------------*/
struct VS_PHONG_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
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
    
    float4 LightViewPosition : TEXCOORD1;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_LIGHT_CUBE_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: PS_LIGHT_CUBE_INPUT definition (remove the comment)
--------------------------------------------------------------------*/
struct PS_LIGHT_CUBE_INPUT
{
    float4 Position : SV_POSITION;
};



//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Vertex Shader function VSPhong definition (remove the comment)
--------------------------------------------------------------------*/
PS_PHONG_INPUT VSPhong(VS_PHONG_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT) 0;

    // perform space transformations
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    // compute the world normal
    // normal vector in world space
    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);

    if (HasNormalMap)
    {
        // Transform tangent and bitangent vector to world space
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }
    
    // Compute the each fragment¡¯s distance to the light source
    //output.LightViewPosition = mul(input.Position, World);
    //output.LightViewPosition = mul(output.LightViewPosition, LightViews[0]);
    //output.LightViewPosition = mul(output.LightViewPosition, LightProjections[0]);

    // position of the vertex in world space
    output.WorldPosition = mul(input.Position, World);

    // texture coordinate
    output.TexCoord = input.TexCoord;

    return output;
}


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE))) / FAR_PLANE;
}


/*--------------------------------------------------------------------
  TODO: Vertex Shader function VSLightCube definition (remove the comment)
--------------------------------------------------------------------*/
PS_LIGHT_CUBE_INPUT VSLightCube( VS_PHONG_INPUT input )
{
    PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT)0;
    output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
    
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
    float3 distanceSquared = float3(0.0f, 0.0f, 0.0f);
    float attenuation[NUM_LIGHTS];
    
    for (uint light = 0u; light < NUM_LIGHTS; ++light)
    {
        float distanceToLight = distance(LightPositions[light].xyz, input.WorldPosition);
        distanceSquared += dot(distanceToLight, distanceToLight);
        attenuation[light] = AttenuationDistance[light].zw / (distanceSquared + 0.000001f);
    }
        
    float3 normal = normalize(input.Normal);
    if (HasNormalMap)
    {
        // Sample the pixel in the normal map.
        float4 bumpMap = normalTexture.Sample(normalSamplers, input.TexCoord);
    
        // Expand the range of the normal value from (0, +1) to (-1, +1).
        bumpMap = (bumpMap * 2.0f) - 1.0f;
    
        // Calculate the normal from the data in the normal map.
        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);
    
        // Normalize the resulting bump normal and replace existing normal
        normal = normalize(bumpNormal);
    }

    float4 TextureColor = diffuseTexture.Sample(diffuseSamplers, input.TexCoord);
    float2 depthTexCoord = float2(0.0f, 0.0f);
    depthTexCoord.x = input.LightViewPosition.x / input.LightViewPosition.w / 2.0f + 0.5f;
    depthTexCoord.y = -input.LightViewPosition.y / input.LightViewPosition.w / 2.0f + 0.5f;

    // Get the closest depth value z from depth map
    float closestDepth = shadowMapTexture.Sample(shadowMapSampler, depthTexCoord).r;
    // Compute the current depth value d
    float currentDepth = input.LightViewPosition.z / input.LightViewPosition.w;
    
    // Linearize the depth values
    closestDepth = LinearizeDepth(closestDepth);
    currentDepth = LinearizeDepth(currentDepth);
    
    float3 ambient = float3(0.1f, 0.1f, 0.1f) * TextureColor.rgb;
    
    // Compare the depth values 
    if (currentDepth > closestDepth + 0.001f)
    {
        // If shadowed, the pixel gets only ambient light
        return float4(ambient, 1.0f);
    }
    
    // ambient
    // float3 ambient = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0u; i < NUM_LIGHTS; ++i)
    {
        ambient += float4(float3(0.1f, 0.1f, 0.1f) * LightColors[i].xyz, 1.0f) * attenuation[i];
    }
    
    // diffuse
    float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    for (uint j = 0u; j < NUM_LIGHTS; ++j)
    {
        lightDirection = normalize(LightPositions[j].xyz - input.WorldPosition);
        diffuse += saturate(dot(normal, lightDirection)) * LightColors[j] * attenuation[j];
    }

    // specular
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 reflectDirection = float3(0.0f, 0.0f, 0.0f);
    float shiness = 20.0f;
    for (uint k = 0; k < NUM_LIGHTS; ++k)
    {
        lightDirection = normalize(LightPositions[k].xyz - input.WorldPosition);
        reflectDirection = reflect(-lightDirection, normal);
        specular += pow(saturate(dot(reflectDirection, viewDirection)), shiness) * LightColors[k] * attenuation[k];
    }

    // Implement phong shading
    return float4(ambient + diffuse + specular, 1.0f) * TextureColor;
        
    // return float4((normal + 1.0f) / 2.0f, 1.0f);
    // return float4((normalize(viewDirection) + 1.0f)/2.0f, 1.0f);    
    // return float4(specular, 1.0f);

}



/*--------------------------------------------------------------------
  TODO: Pixel Shader function PSLightCube definition (remove the comment)
--------------------------------------------------------------------*/
float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
    return OutputColor;
}