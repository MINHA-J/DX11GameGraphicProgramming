#include "Model/Model.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		    // output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   ConvertMatrix
      Summary:  Convert aiMatrix4x4 to XMMATRIX
      Returns:  XMMATRIX
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    XMMATRIX ConvertMatrix(_In_ const aiMatrix4x4& matrix)
    {
        return XMMATRIX(
            matrix.a1,
            matrix.b1,
            matrix.c1,
            matrix.d1,
            matrix.a2,
            matrix.b2,
            matrix.c2,
            matrix.d2,
            matrix.a3,
            matrix.b3,
            matrix.c3,
            matrix.d3,
            matrix.a4,
            matrix.b4,
            matrix.c4,
            matrix.d4
        );
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   ConvertVector3dToFloat3
      Summary:  Conver aiVector3D to XMFLOAT3
      Returns:  XMFLOAT3
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    XMFLOAT3 ConvertVector3dToFloat3(_In_ const aiVector3D& vector)
    {
        return XMFLOAT3(vector.x, vector.y, vector.z);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   ConvertQuaternionToVector
      Summary:  Convert aiQuaternion to XMVECTOR
      Returns:  XMVECTOR
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    XMVECTOR ConvertQuaternionToVector(_In_ const aiQuaternion& quaternion)
    {
        XMFLOAT4 float4 = XMFLOAT4(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
        return XMLoadFloat4(&float4);
    }

    std::unique_ptr<Assimp::Importer> Model::sm_pImporter = std::make_unique<Assimp::Importer>();

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::Model
      Summary:  Constructor
      Args:     const std::filesystem::path& filePath
                  Path to the model to load
      Modifies: [m_filePath, m_animationBuffer, m_skinningConstantBuffer,
                 m_skinningConstantBuffer, m_aVertices, m_aAnimationData,
                 m_aIndices, m_aBoneData, m_aBoneInfo, m_aTransforms,
                 m_aBoneInfo, m_aTransforms, m_boneNameToIndexMap,
                 m_pScene, m_timeSinceLoaded, m_globalInverseTransform].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Model::Model(_In_ const std::filesystem::path& filePath)
        : Renderable(XMFLOAT4(1.0, 1.0, 1.0, 1.0))
        , m_filePath(filePath)
        , m_animationBuffer(nullptr)
        , m_skinningConstantBuffer(nullptr)
        , m_aVertices(std::vector<SimpleVertex>())
        , m_aAnimationData(std::vector<AnimationData>())
        , m_aIndices(std::vector<WORD>())
        , m_aBoneData(std::vector<VertexBoneData>())
        , m_aBoneInfo(std::vector<BoneInfo>())
        , m_aTransforms(std::vector<XMMATRIX>())
        , m_boneNameToIndexMap(std::unordered_map<std::string, UINT>())
        , m_pScene()
        , m_timeSinceLoaded(0.0f)
        , m_globalInverseTransform()
    { }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::Initialize
      Summary:  Load and initialize the 3d model and create buffers
      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
      Modifies: [m_pScene, m_globalInverseTransform, m_animationBuffer,
                 m_skinningConstantBuffer].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
    {
        HRESULT hr = S_OK;

        // Read the 3d model file using Assimp importer(m_importer), store thre read scene into m_pScene
        //m_pScene = sm_pImporter->ReadFile(
        //    m_filePath.string().c_str(),
        //    ASSIMP_LOAD_FLAGS
        //);

        // With Assimp, we can calculate T and B vectors easily when importing the model
        m_pScene = sm_pImporter->ReadFile(
            m_filePath.string().c_str(),
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded
        );
        
        if (m_pScene != nullptr)
        {
            // set m_globalInverseTransform as matrix from world space to model space
            m_globalInverseTransform = ConvertMatrix(m_pScene->mRootNode->mTransformation);
            XMVECTOR det = XMMatrixDeterminant(m_globalInverseTransform);
            m_globalInverseTransform = XMMatrixInverse(&det, m_globalInverseTransform);

            // Initialize the model
            hr = initFromScene(pDevice, pImmediateContext, m_pScene, m_filePath);
            if (FAILED(hr))
                return hr;
        }
        else
        {
            hr = E_FAIL;
            OutputDebugString(L"Error parsing ");
            OutputDebugString(m_filePath.c_str());
            OutputDebugString(L": ");
            OutputDebugStringA(sm_pImporter->GetErrorString());
            OutputDebugString(L"\n");

            return hr;
        }

        // Create the vertex buffer, m_animationBuffer 
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(AnimationData) * (UINT)m_aAnimationData.size();
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA InitData = {}; // with initial data m_aAnimationData
        InitData.pSysMem = m_aAnimationData.data(); 
        hr = pDevice->CreateBuffer(&bd, &InitData, m_animationBuffer.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create the constant buffer, m_skinningConstantBuffer
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CBSkinning);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;
        hr = pDevice->CreateBuffer(&bd, nullptr, m_skinningConstantBuffer.GetAddressOf());
        if (FAILED(hr))
            return hr;

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::Update
      Summary:  Update bone transformations
      Args:     FLOAT deltaTime
                  Time difference of a frame
      Modifies: [m_aTransforms].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::Update(_In_ FLOAT deltaTime)
    {
        // Update m_timeSinceLoaded by adding delta time
        m_timeSinceLoaded += deltaTime;

        // If m_pScene has animations
        if (m_pScene->HasAnimations())
        {
            // Calculate the current animation time to play, using ticks per second and duration of animation
            XMMATRIX identity = XMMatrixIdentity();
            FLOAT ticksPerSecond = static_cast<FLOAT>(m_pScene->mAnimations[0]->mTicksPerSecond != 0.0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
            FLOAT timeInTicks = m_timeSinceLoaded * ticksPerSecond;
            FLOAT animationTimeTicks = fmod(timeInTicks, static_cast<FLOAT>(m_pScene->mAnimations[0]->mDuration));

            // If root node of m_pScene is valid
            if (m_pScene->mRootNode != NULL)
            {
                // Calculate the bone transform matrices
                readNodeHierarchy(animationTimeTicks, m_pScene->mRootNode, identity);

                // Resize m_aTransforms same as m_aBoneInfo
                m_aTransforms.resize(m_aBoneInfo.size());

                // Store each final transformations in the bone information to the m_aTransforms
                for (UINT i = 0; i < m_aBoneInfo.size(); ++i)
                {
                    m_aTransforms[i] = m_aBoneInfo[i].FinalTransformation;
                }
            }
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::GetAnimationBuffer
      Summary:  Returns the animation buffer
      Returns:  ComPtr<ID3D11Buffer>&
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Model::GetAnimationBuffer()
    {
        return m_animationBuffer;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::GetSkinningConstantBuffer
      Summary:  Returns the skinning constant buffer
      Returns:  ComPtr<ID3D11Buffer>&
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Model::GetSkinningConstantBuffer()
    {
        return m_skinningConstantBuffer;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::GetNumVertices

      Summary:  Returns the number of vertices

      Returns:  UINT
                  Number of vertices
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    UINT Model::GetNumVertices() const
    {
        return static_cast<UINT>(m_aVertices.size());
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::GetNumIndices

      Summary:  Returns the number of indices

      Returns:  UINT
                  Number of indices
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    UINT Model::GetNumIndices() const
    {
        return static_cast<UINT>(m_aIndices.size());
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
       Method:   Model::GetBoneTransforms
       Summary:  Returns the vector containing bone transforms
       Returns:  std::vector<XMMATRIX>&
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::vector<XMMATRIX>& Model::GetBoneTransforms()
    {
        return m_aTransforms;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::GetBoneNameToIndexMap
        Summary:  Returns the bone name to index map
        Returns:  std::unordered_map<std::string, UINT>&
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const std::unordered_map<std::string, UINT>& Model::GetBoneNameToIndexMap() const
    {
        return m_boneNameToIndexMap;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::countVerticesAndIndices

      Summary:  Fill the BasicMeshEntry information

      Args:     UINT& uOutNumVertices
                  Total number of vertices
                UINT& uOutNumIndices
                  Total number of indices
                const aiScene* pScene
                  Pointer to an assimp scene object that contains the
                  mesh information
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::countVerticesAndIndices(
        _Inout_ UINT& uOutNumVertices,
        _Inout_ UINT& uOutNumIndices,
        _In_ const aiScene* pScene)
    {
        // Based on the scene's meshes, load the information per mesh
        for (UINT i = 0u; i < m_aMeshes.size(); ++i)
        {
            // Material Index
            m_aMeshes[i].uMaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
            // Number of indices
            m_aMeshes[i].uNumIndices = pScene->mMeshes[i]->mNumFaces * 3u;
            // Base Vertex
            m_aMeshes[i].uBaseVertex = uOutNumVertices;
            // Base Index
            m_aMeshes[i].uBaseIndex = uOutNumIndices;

            // Count the total number of vertices and indices
            uOutNumVertices += pScene->mMeshes[i]->mNumVertices;
            uOutNumIndices += m_aMeshes[i].uNumIndices;
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::findNodeAnimOrNull
        Summary:  Find the aiNodeAnim with the givne node name in the given animation
        Args:     const aiAnimation* pAnimation
                    Pointer to an assimp animation object
                  PCSTR pszNodeName
                    Node name to find

        Returns:  aiNodeAnim* or nullptr
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const aiNodeAnim* Model::findNodeAnimOrNull(_In_ const aiAnimation* pAnimation, _In_ PCSTR pszNodeName)
    {
        for (UINT i = 0u; i < pAnimation->mNumChannels; ++i)
        {
            const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

            if (strncmp(pNodeAnim->mNodeName.data, pszNodeName, pNodeAnim->mNodeName.length) == 0)
            {
                return pNodeAnim;
            }
        }

        return nullptr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::findPosition
        Summary:  Find the index of the position key right before the given animation time
        Args:     FLOAT animationTimeTicks
                    Animation time
                  const aiNodeAnim* pNodeAnim
                     Pointer to an assimp node anim object
        Returns:  UINT
                    Index of the key
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    UINT Model::findPosition(_In_ FLOAT animationTimeTicks, _In_ const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumPositionKeys > 0);

        for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; ++i)
        {
            FLOAT t = static_cast<FLOAT>(pNodeAnim->mPositionKeys[i + 1].mTime);

            if (animationTimeTicks < t)
            {
                return i;
            }
        }

        return 0u;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::findRotation
        Summary:  Find the index of the rotation key right before the given animation time
        Args:     FLOAT animationTimeTicks
                    Animation time
                  const aiNodeAnim* pNodeAnim
                     Pointer to an assimp node anim object
        Returns:  UINT
                    Index of the key
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    UINT Model::findRotation(_In_ FLOAT animationTimeTicks, _In_ const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumRotationKeys > 0);

        for (UINT i = 0u; i < pNodeAnim->mNumRotationKeys - 1; ++i)
        {
            FLOAT t = static_cast<FLOAT>(pNodeAnim->mRotationKeys[i + 1].mTime);

            if (animationTimeTicks < t)
            {
                return i;
            }
        }

        return 0u;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::findScaling
        Summary:  Find the index of the scaling key right before the given animation time
        Args:     FLOAT animationTimeTicks
                    Animation time
                  const aiNodeAnim* pNodeAnim
                     Pointer to an assimp node anim object
        Returns:  UINT
                    Index of the key
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    UINT Model::findScaling(_In_ FLOAT animationTimeTicks, _In_ const aiNodeAnim* pNodeAnim)
    {
        assert(pNodeAnim->mNumScalingKeys > 0);

        for (UINT i = 0u; i < pNodeAnim->mNumScalingKeys - 1; ++i)
        {
            FLOAT t = static_cast<FLOAT>(pNodeAnim->mScalingKeys[i + 1].mTime);

            if (animationTimeTicks < t)
            {
                return i;
            }
        }

        return 0u;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::getBoneId
        Summary:  Find the the index of the bone
        Args:      const aiBone* pBone
                     Pointer to an assimp bone object
        Modifies: [m_boneNameToIndexMap].
        Returns:  UINT
                    Index of the bone
     M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    UINT Model::getBoneId(_In_ const aiBone* pBone)
    {
        UINT uBoneIndex = 0u;
        PCSTR pszBoneName = pBone->mName.C_Str();
        if (!m_boneNameToIndexMap.contains(pszBoneName))
        {
            uBoneIndex = static_cast<UINT>(m_boneNameToIndexMap.size());
            m_boneNameToIndexMap[pszBoneName] = uBoneIndex;
        }
        else
        {
            uBoneIndex = m_boneNameToIndexMap[pszBoneName];
        }

        return uBoneIndex;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::getVertices

      Summary:  Returns the vertices data

      Returns:  const SimpleVertex*
                  Array of vertices
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const SimpleVertex* Model::getVertices() const
    {
        return m_aVertices.data();
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::getIndices

      Summary:  Returns the indices data

      Returns:  const WORD*
                  Array of indices
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const WORD* Model::getIndices() const
    {
        return m_aIndices.data();
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::getIndices

      Summary:  Initialize all meshes in a given assimp scene

      Args:     const aiScene* pScene
                  Assimp scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::initAllMeshes(_In_ const aiScene* pScene)
    {
        // Initialize all meshes in the scene
        for (UINT i = 0u; i < m_aMeshes.size(); ++i)
        {
            const aiMesh* pMesh = pScene->mMeshes[i];
            initSingleMesh(i, pMesh);
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::initFromScene

      Summary:  Initialize all meshes in a given assimp scene

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                const aiScene* pScene
                  Assimp scene
                const std::filesystem::path& filePath
                  Path to the model

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::initFromScene(
        _In_ ID3D11Device* pDevice,
        _In_ ID3D11DeviceContext* pImmediateContext,
        _In_ const aiScene* pScene,
        _In_ const std::filesystem::path& filePath
    )
    {
        // Based on the number of meshes and materials, resize the meshes vector and materials vector accordingly
        m_aMeshes.resize(pScene->mNumMeshes);
        // m_aMaterials.resize(pScene->mNumMaterials);

        UINT uNumVertices = 0u;
        UINT uNumIndices = 0u;
        HRESULT hr = S_OK;

        // Load the mesh information to get the total number of vertices / indices(countVerticesAndIndices) 
        countVerticesAndIndices(uNumVertices, uNumIndices, pScene);

        // Reserve spaces for the verticesand indices data(reserveSpace)
        reserveSpace(uNumVertices, uNumIndices);

        // Initialize the meshes(initAllMeshes)
        initAllMeshes(pScene);

        // Initialize the materials(initMaterials)
        hr = initMaterials(pDevice, pImmediateContext, pScene, filePath);
        if (FAILED(hr))
            return hr;

        for (UINT i = 0; i < m_aBoneData.size(); ++i)
        {
            // Create AnimationData for the vertex and add it to m_aAnimationData
            m_aAnimationData.push_back(
                AnimationData
                {
                    .aBoneIndices = (XMUINT4)m_aBoneData[i].aBoneIds,
                    .aBoneWeights = (XMFLOAT4)m_aBoneData[i].aWeights
                }
            );
        }

        // Initialize the buffers(initialize)
        hr = initialize(pDevice, pImmediateContext);
        if (FAILED(hr))
            return hr;

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::initMaterials

      Summary:  Initialize all materials in a given assimp scene

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                const aiScene* pScene
                  Assimp scene
                const std::filesystem::path& filePath
                  Path to the model

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::initMaterials(
        _In_ ID3D11Device* pDevice,
        _In_ ID3D11DeviceContext* pImmediateContext,
        _In_ const aiScene* pScene,
        _In_ const std::filesystem::path& filePath
    )
    {
        HRESULT hr = S_OK;

        // Extract the directory part from the file name
        std::filesystem::path parentDirectory = filePath.parent_path();

        // Initialize the materials
        for (UINT i = 0u; i < pScene->mNumMaterials; ++i)
        {
            const aiMaterial* pMaterial = pScene->mMaterials[i];

            std::string szName = filePath.string() + std::to_string(i);
            std::wstring pwszName(szName.length(), L' ');
            std::copy(szName.begin(), szName.end(), pwszName.begin());
            m_aMaterials.push_back(std::make_shared<Material>(pwszName));

            loadTextures(pDevice, pImmediateContext, parentDirectory, pMaterial, i);
        }

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::initMeshBones
        Summary:  Initialize all bones in a given aiMesh
        Args:     const aiScene* pScene
                    Assimp scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::initMeshBones(_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh)
    {
        // For each bone of the given mesh
        for (UINT i = 0; i < pMesh->mNumBones; ++i)
        {
            // const aiBone* pBone = pMesh[uMeshIndex].mBones[i];
            // Call initMeshSingleBone
            initMeshSingleBone(uMeshIndex, pMesh->mBones[i]);
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   Model::initMeshSingleBone
        Summary:  Initialize a single bone of the mesh
        Args:     const aiScene* pScene
                    Assimp scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::initMeshSingleBone(_In_ UINT uMeshIndex, _In_ const aiBone* pBone)
    {
        UINT uBoneId = getBoneId(pBone);

        if (uBoneId == m_aBoneInfo.size())
        {
            BoneInfo boneInfo(ConvertMatrix(pBone->mOffsetMatrix));
            m_aBoneInfo.push_back(boneInfo);
        }

        for (UINT i = 0u; i < pBone->mNumWeights; ++i)
        {
            const aiVertexWeight& vertexWeight = pBone->mWeights[i];
            UINT uGlobalVertexId = m_aMeshes[uMeshIndex].uBaseVertex + vertexWeight.mVertexId;
            m_aBoneData[uGlobalVertexId].AddBoneData(uBoneId, vertexWeight.mWeight);
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::initSingleMesh

      Summary:  Initialize single mesh from a given assimp mesh

      Args:     const aiMesh* pMesh
                  Point to an assimp mesh object
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::initSingleMesh(_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh)
    {
        const aiVector3D zero3d(0.0f, 0.0f, 0.0f);

        // Populate the vertex attribute vectors
        for (UINT i = 0u; i < pMesh->mNumVertices; ++i) // For each vertex in the mesh
        {
            const aiVector3D& position = pMesh->mVertices[i];
            const aiVector3D& normal = pMesh->mNormals[i];
            const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ? pMesh->mTextureCoords[0][i] : zero3d;

            m_aVertices.push_back(
                SimpleVertex
                {
                    .Position = XMFLOAT3(position.x, position.y, position.z),
                    .TexCoord = XMFLOAT2(texCoord.x, texCoord.y),
                    .Normal = XMFLOAT3(normal.x, normal.y, normal.z)
                }
            );
            
            const aiVector3D& tangent = pMesh->HasTangentsAndBitangents() ? pMesh->mTangents[i] : zero3d;
            const aiVector3D& bitangent = pMesh->HasTangentsAndBitangents() ? pMesh->mBitangents[i] : zero3d;

            m_aNormalData.push_back(
                NormalData
                {
                    .Tangent = XMFLOAT3(tangent.x, tangent.y, tangent.z),
                    .Bitangent = XMFLOAT3(bitangent.x, bitangent.y, bitangent.z)
                }
            );

        }

        // call initMeshBones
        initMeshBones(uMeshIndex, pMesh);

        // For each face in the mesh, Populate the index buffer
        for (UINT i = 0u; i < pMesh->mNumFaces; ++i)
        {
            const aiFace& face = pMesh->mFaces[i];
            assert(face.mNumIndices == 3u);

            m_aIndices.push_back(static_cast<WORD>(face.mIndices[0]));
            m_aIndices.push_back(static_cast<WORD>(face.mIndices[1]));
            m_aIndices.push_back(static_cast<WORD>(face.mIndices[2]));
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::interpolatePosition
      Summary:  Interpolate two keyframes to find translate vector
      Args:     XMFLOAT3& outTranslate
                  Translate vector
                FLOAT animationTimeTicks
                  Animation time
                const aiNodeAnim* pNodeAnim
                  Pointer to an assimp node anim object
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::interpolatePosition(_Inout_ XMFLOAT3& outTranslate, _In_ FLOAT animationTimeTicks, _In_ const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumPositionKeys == 1)
        {
            outTranslate = ConvertVector3dToFloat3(pNodeAnim->mPositionKeys[0].mValue);
            return;
        }

        UINT uPositionIndex = findPosition(animationTimeTicks, pNodeAnim);
        UINT uNextPositionIndex = uPositionIndex + 1u;
        assert(uNextPositionIndex < pNodeAnim->mNumPositionKeys);

        FLOAT t1 = static_cast<FLOAT>(pNodeAnim->mPositionKeys[uPositionIndex].mTime);
        FLOAT t2 = static_cast<FLOAT>(pNodeAnim->mPositionKeys[uNextPositionIndex].mTime);
        FLOAT deltaTime = t2 - t1;
        FLOAT factor = (animationTimeTicks - t1) / deltaTime;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiVector3D& start = pNodeAnim->mPositionKeys[uPositionIndex].mValue;
        const aiVector3D& end = pNodeAnim->mPositionKeys[uNextPositionIndex].mValue;
        aiVector3D delta = end - start;
        outTranslate = ConvertVector3dToFloat3(start + factor * delta);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::interpolateRotation
      Summary:  Interpolate two keyframes to find rotation vector
      Args:     XMVECTOR& outQuaternion
                  Quaternion vector
                FLOAT animationTimeTicks
                  Animation time
                const aiNodeAnim* pNodeAnim
                  Pointer to an assimp node anim object
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::interpolateRotation(_Inout_ XMVECTOR& outQuaternion, _In_ FLOAT animationTimeTicks, _In_ const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumRotationKeys == 1)
        {
            outQuaternion = ConvertQuaternionToVector(pNodeAnim->mRotationKeys[0].mValue);
            return;
        }

        // This function finds rotation vector by interpolating the two keyframes
        UINT uRotationIndex = findRotation(animationTimeTicks,pNodeAnim);
        UINT uNextRotationIndex = uRotationIndex + 1u;
        assert(uNextRotationIndex < pNodeAnim->mNumRotationKeys);

        FLOAT t1 = static_cast<FLOAT>(pNodeAnim->mRotationKeys[uRotationIndex].mTime);
        FLOAT t2 = static_cast<FLOAT>(pNodeAnim->mRotationKeys[uNextRotationIndex].mTime);
        FLOAT deltaTime = t2 - t1;
        FLOAT factor = (animationTimeTicks - t1) / deltaTime;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiQuaternion& start = pNodeAnim->mRotationKeys[uRotationIndex].mValue;
        const aiQuaternion& end = pNodeAnim->mRotationKeys[uNextRotationIndex].mValue;

        aiQuaternion ret;
        aiQuaternion::Interpolate(ret, start, end, factor);
        ret = start;
        ret.Normalize();
        outQuaternion = ConvertQuaternionToVector(ret);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::interpolateScaling
      Summary:  Interpolate two keyframes to find rotation vector
      Args:     XMFLOAT3& outScale
                  Scaling vector
                FLOAT animationTimeTicks
                  Animation time
                const aiNodeAnim* pNodeAnim
                  Pointer to an assimp node anim object
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::interpolateScaling(_Inout_ XMFLOAT3& outScale, _In_ FLOAT animationTimeTicks, _In_ const aiNodeAnim* pNodeAnim)
    {
        if (pNodeAnim->mNumScalingKeys == 1)
        {
            outScale = ConvertVector3dToFloat3(pNodeAnim->mScalingKeys[0].mValue);
            return;
        }
        // This function finds scaling vector by interpolating the two keyframes
        UINT uScalingIndex = findScaling(animationTimeTicks, pNodeAnim);
        UINT uNextScalingIndex = uScalingIndex + 1u;
        assert(uNextScalingIndex < pNodeAnim->mNumScalingKeys);

        FLOAT t1 = static_cast<FLOAT>(pNodeAnim->mScalingKeys[uScalingIndex].mTime);
        FLOAT t2 = static_cast<FLOAT>(pNodeAnim->mScalingKeys[uNextScalingIndex].mTime);
        FLOAT deltaTime = t2 - t1;
        FLOAT factor = (animationTimeTicks - t1) / deltaTime;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiVector3D& start = pNodeAnim->mScalingKeys[uScalingIndex].mValue;
        const aiVector3D& end = pNodeAnim->mScalingKeys[uNextScalingIndex].mValue;
        aiVector3D delta = end - start;
        outScale = ConvertVector3dToFloat3(start + factor * delta);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::loadDiffuseTexture

      Summary:  Load a diffuse texture from given path

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                const std::filesystem::path& parentDirectory
                  Parent path to the model
                const aiMaterial* pMaterial
                  Pointer to an assimp material object
                UINT uIndex
                  Index to a material
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::loadDiffuseTexture(
        _In_ ID3D11Device* pDevice,
        _In_ ID3D11DeviceContext* pImmediateContext,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial,
        _In_ UINT uIndex
    )
    {
        HRESULT hr = S_OK;
        m_aMaterials[uIndex]->pDiffuse = nullptr;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString aiPath;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
            {
                std::string szPath(aiPath.data);

                if (szPath.substr(0ull, 2ull) == ".\\")
                {
                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
                }

                std::filesystem::path fullPath = parentDirectory / szPath;

                m_aMaterials[uIndex]->pDiffuse = std::make_shared<Texture>(fullPath);

                hr = m_aMaterials[uIndex]->pDiffuse->Initialize(pDevice, pImmediateContext);
                if (FAILED(hr))
                {
                    OutputDebugString(L"Error loading diffuse texture \"");
                    OutputDebugString(fullPath.c_str());
                    OutputDebugString(L"\"\n");

                    return hr;
                }

                OutputDebugString(L"Loaded diffuse texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
            }
        }

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::loadSpecularTexture

      Summary:  Load a specular texture from given path

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                const std::filesystem::path& parentDirectory
                  Parent path to the model
                const aiMaterial* pMaterial
                  Pointer to an assimp material object
                UINT uIndex
                  Index to a material
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::loadSpecularTexture(
        _In_ ID3D11Device* pDevice,
        _In_ ID3D11DeviceContext* pImmediateContext,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial,
        _In_ UINT uIndex
    )
    {
        HRESULT hr = S_OK;
        m_aMaterials[uIndex]->pSpecularExponent = nullptr;

        if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
        {
            aiString aiPath;

            if (pMaterial->GetTexture(aiTextureType_SHININESS, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
            {
                std::string szPath(aiPath.data);

                if (szPath.substr(0ull, 2ull) == ".\\")
                {
                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
                }

                std::filesystem::path fullPath = parentDirectory / szPath;

                m_aMaterials[uIndex]->pSpecularExponent = std::make_shared<Texture>(fullPath);

                hr = m_aMaterials[uIndex]->pSpecularExponent->Initialize(pDevice, pImmediateContext);
                if (FAILED(hr))
                {
                    OutputDebugString(L"Error loading specular texture \"");
                    OutputDebugString(fullPath.c_str());
                    OutputDebugString(L"\"\n");

                    return hr;
                }

                OutputDebugString(L"Loaded specular texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
            }
        }

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::loadNormalTexture
      Summary:  Load a normal texture from given path
      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                const std::filesystem::path& parentDirectory
                  Parent path to the model
                const aiMaterial* pMaterial
                  Pointer to an assimp material object
                UINT uIndex
                  Index to a material
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::loadNormalTexture(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext, _In_ const std::filesystem::path& parentDirectory, _In_ const aiMaterial* pMaterial, _In_ UINT uIndex)
    {
        HRESULT hr = S_OK;
        m_aMaterials[uIndex]->pNormal = nullptr;

        if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            aiString aiPath;

            if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
            {
                std::string szPath(aiPath.data);

                if (szPath.substr(0ull, 2ull) == ".\\")
                {
                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
                }

                std::filesystem::path fullPath = parentDirectory / szPath;

                m_aMaterials[uIndex]->pNormal = std::make_shared<Texture>(fullPath);
                m_bHasNormalMap = true;

                if (FAILED(hr))
                {
                    OutputDebugString(L"Error loading normal texture \"");
                    OutputDebugString(fullPath.c_str());
                    OutputDebugString(L"\"\n");

                    return hr;
                }

                OutputDebugString(L"Loaded normal texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
            }
        }

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::loadTextures

      Summary:  Load a specular texture from given path

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers
                const std::filesystem::path& parentDirectory
                  Parent path to the model
                const aiMaterial* pMaterial
                  Pointer to an assimp material object
                UINT uIndex
                  Index to a material
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Model::loadTextures(
        _In_ ID3D11Device* pDevice,
        _In_ ID3D11DeviceContext* pImmediateContext,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial,
        _In_ UINT uIndex
    )
    {
        HRESULT hr = loadDiffuseTexture(pDevice, pImmediateContext, parentDirectory, pMaterial, uIndex);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = loadSpecularTexture(pDevice, pImmediateContext, parentDirectory, pMaterial, uIndex);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = loadNormalTexture(pDevice, pImmediateContext, parentDirectory, pMaterial, uIndex);
        if (FAILED(hr))
        {
            return hr;
        }

        return hr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::readNodeHierarchy
      Summary:  Calculate bone transformation of the given assimp node
      Args:     FLOAT animationTimeTicks
                  Animation time
               const aiNode* pNode
                  Pointer to an assimp node object
                const XMMATRIX& parentTransform
                  Parent transform in hierarchy
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::readNodeHierarchy(_In_ FLOAT animationTimeTicks, _In_ const aiNode* pNode, _In_ const XMMATRIX& parentTransform)
    {
        // This function is a recursive function, We will play the 0th animation of the animations that aiScene has

        PCSTR pszNodeName = pNode->mName.C_Str();
        XMMATRIX NodeTransform = ConvertMatrix(pNode->mTransformation);

        // Find current aiNodeAnim
        const aiNodeAnim* pNodeAnim = findNodeAnimOrNull(m_pScene->mAnimations[0], pszNodeName);
        
        if (pNodeAnim != nullptr)
        {
            // Calculate scaling matrix, rotation matrix, translation matrix
            XMFLOAT3 outTranslate = { 0.f, 0.f, 0.f};
            XMVECTOR outQuaternion = { 0.f, 0.f, 0.f, 0.f };
            XMFLOAT3 outScale = { 0.f, 0.f, 0.f };

            interpolatePosition(outTranslate, animationTimeTicks, pNodeAnim);
            interpolateRotation(outQuaternion, animationTimeTicks, pNodeAnim);
            interpolateScaling(outScale, animationTimeTicks, pNodeAnim);

            XMMATRIX translationMatrix = XMMatrixTranslation(outTranslate.x, outTranslate.y, outTranslate.z);
            XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(outQuaternion);
            XMMATRIX scalingMatrix = XMMatrixScaling(outScale.x, outScale.y, outScale.z);
            
            // Replace NodeTransform with final transformation matrix
            NodeTransform = scalingMatrix  * rotationMatrix * translationMatrix;      
            // NodeTransform = translationMatrix * rotationMatrix * scalingMatrix;
        }

        XMMATRIX globalTransformation = NodeTransform * parentTransform;
        // XMMATRIX globalTransformation = parentTransform * NodeTransform;
        
        // If m_boneNameToIndexMap has the key that matches the name of the given node
        if (m_boneNameToIndexMap.find(pszNodeName) != m_boneNameToIndexMap.end())
        {
            // Get the bone index and retrieve the boneInfo in m_aBoneInfo
            UINT boneIndex = m_boneNameToIndexMap[pszNodeName];

            // Set the FinalTransformation of boneInfo properly
            m_aBoneInfo[boneIndex].FinalTransformation = m_aBoneInfo[boneIndex].OffsetMatrix * globalTransformation * m_globalInverseTransform;
            // m_aBoneInfo[boneIndex].FinalTransformation =m_globalInverseTransform  * globalTransformation * m_aBoneInfo[boneIndex].OffsetMatrix;
        }

        // For each child of current node
        for (UINT i = 0; i < pNode->mNumChildren; i++)
        {
            readNodeHierarchy(animationTimeTicks, pNode->mChildren[i], globalTransformation); 
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Model::reserveSpace

      Summary:  Reserve space for vertices and indices vectors

      Args:     UINT uNumVertices
                  Number of vertices
                UINT uNumIndices
                  Number of indices
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Model::reserveSpace(_In_ UINT uNumVertices, _In_ UINT uNumIndices)
    {
        // Reserve spaces of the vertices / indices vector according to the number of vertices and indices
        m_aVertices.reserve(uNumVertices);
        m_aIndices.reserve(uNumIndices);
        m_aBoneData.resize(uNumVertices);
    }

}