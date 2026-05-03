#include "Importers/FBXImporterInternal.h"
#include <mikktspace.h>

namespace pulsared
{
    // -----------------------------------------------------------------------
    // Mikktspace 切线生成
    // -----------------------------------------------------------------------
    struct MikkTangentGenData
    {
        const array_list<Vector3f>* Positions;
        const array_list<Vector3f>* Normals;
        const array_list<Vector2f>* UVs;
        array_list<Vector4f>*       Tangents;
    };

    static int MikkGetNumFaces(const SMikkTSpaceContext* pContext)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        return static_cast<int>(data->Positions->size() / 3);
    }

    static int MikkGetNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
    {
        return 3;
    }

    static void MikkGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        const Vector3f& p = (*data->Positions)[iFace * 3 + iVert];
        fvPosOut[0] = p.x; fvPosOut[1] = p.y; fvPosOut[2] = p.z;
    }

    static void MikkGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        const Vector3f& n = (*data->Normals)[iFace * 3 + iVert];
        fvNormOut[0] = n.x; fvNormOut[1] = n.y; fvNormOut[2] = n.z;
    }

    static void MikkGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        const Vector2f& uv = (*data->UVs)[iFace * 3 + iVert];
        fvTexcOut[0] = uv.x; fvTexcOut[1] = uv.y;
    }

    static void MikkSetTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
    {
        auto* data = static_cast<MikkTangentGenData*>(pContext->m_pUserData);
        (*data->Tangents)[iFace * 3 + iVert] = Vector4f{fvTangent[0], fvTangent[1], fvTangent[2], fSign};
    }

    void GenerateTangentsForSection(
        const array_list<Vector3f>& positions,
        const array_list<Vector3f>& normals,
        const array_list<Vector2f>& uvs,
        array_list<Vector4f>& outTangents)
    {
        if (uvs.empty()) return;
        if (positions.empty()) return;

        outTangents.resize(positions.size());

        MikkTangentGenData data{&positions, &normals, &uvs, &outTangents};

        SMikkTSpaceInterface iface{};
        iface.m_getNumFaces          = MikkGetNumFaces;
        iface.m_getNumVerticesOfFace = MikkGetNumVerticesOfFace;
        iface.m_getPosition          = MikkGetPosition;
        iface.m_getNormal            = MikkGetNormal;
        iface.m_getTexCoord          = MikkGetTexCoord;
        iface.m_setTSpaceBasic       = MikkSetTSpaceBasic;

        SMikkTSpaceContext ctx{};
        ctx.m_pInterface = &iface;
        ctx.m_pUserData  = &data;

        genTangSpaceDefault(&ctx);
    }

    // -----------------------------------------------------------------------
    // 简化版切线生成（逐三角计算 + 顶点平均）
    // -----------------------------------------------------------------------
    void GenerateSimpleTangentsForSection(
        const array_list<Vector3f>& positions,
        const array_list<Vector3f>& normals,
        const array_list<Vector2f>& uvs,
        array_list<Vector4f>& outTangents)
    {
        if (uvs.empty() || positions.empty()) return;

        const size_t vertCount = positions.size();
        outTangents.resize(vertCount);
        array_list<Vector3f> accumT(vertCount);
        array_list<float>    accumSign(vertCount, 0.0f);

        const size_t triCount = vertCount / 3;
        for (size_t tri = 0; tri < triCount; ++tri)
        {
            const size_t i0 = tri * 3 + 0;
            const size_t i1 = tri * 3 + 1;
            const size_t i2 = tri * 3 + 2;

            const Vector3f e1  = positions[i1] - positions[i0];
            const Vector3f e2  = positions[i2] - positions[i0];
            const Vector2f duv1 = uvs[i1] - uvs[i0];
            const Vector2f duv2 = uvs[i2] - uvs[i0];

            const float det = duv1.x * duv2.y - duv1.y * duv2.x;
            if (fabsf(det) < FLT_EPSILON) continue;

            const float r = 1.0f / det;
            const Vector3f T = (e1 * duv2.y - e2 * duv1.y) * r;
            const Vector3f B = (e2 * duv1.x - e1 * duv2.x) * r;

            for (int j = 0; j < 3; ++j)
            {
                const size_t idx = tri * 3 + j;
                accumT[idx] = accumT[idx] + T;
                const float w = Dot(Cross(normals[idx], T), B) > 0.0f ? 1.0f : -1.0f;
                accumSign[idx] += w;
            }
        }

        for (size_t i = 0; i < vertCount; ++i)
        {
            Vector3f T = accumT[i];
            if (Dot(T, T) > FLT_EPSILON)
            {
                T = Normalize(T);
                const Vector3f N = normals[i];
                T = Normalize(T - N * Dot(N, T)); // Gram-Schmidt
            }
            else
            {
                T = Vector3f{1, 0, 0};
            }
            const float w = accumSign[i] >= 0.0f ? 1.0f : -1.0f;
            outTangents[i] = Vector4f{T.x, T.y, T.z, w};
        }
    }

    // -----------------------------------------------------------------------
    // ProcessMesh（StaticMesh）
    // -----------------------------------------------------------------------
    RCPtr<StaticMesh> ProcessMesh(FbxNode* fbxNode, bool inverseCoordsystem, bool recomputeTangents, bool useMikktspace)
    {
        const auto name = fbxNode->GetName();

        array_list<StaticMeshSection> sections;
        array_list<string> materialNames;

        materialNames.reserve(fbxNode->GetMaterialCount());
        for (int i = 0; i < fbxNode->GetMaterialCount(); i++)
        {
            materialNames.push_back(fbxNode->GetMaterial(i)->GetName());
        }

        const auto attrCount = fbxNode->GetNodeAttributeCount();
        for (int attrIndex = 0; attrIndex < attrCount; attrIndex++)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(attrIndex);
            if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                StaticMeshSection section;

                auto fbxMesh = static_cast<FbxMesh*>(attr);
                assert(fbxMesh);

                constexpr int kPolygonCount = 3;
                const auto vertexCount = fbxMesh->GetPolygonVertexCount();
                const auto polygonCount = fbxMesh->GetPolygonCount();
                assert(vertexCount == polygonCount * kPolygonCount);

                // 确定实际 UV 套数
                const uint8_t numUV = (uint8_t)std::min(fbxMesh->GetUVLayerCount(), (int)STATICMESH_MAX_TEXTURE_COORDS);
                section.NumTexCoords = numUV;

                // 初始化各属性数组
                section.Indices.resize(vertexCount);
                section.Positions.resize(vertexCount);
                section.Normals.resize(vertexCount);
                section.TexCoords.resize(numUV);
                for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                    section.TexCoords[uvIdx].resize(vertexCount);

                const bool hasColors   = fbxMesh->GetLayer(0) && fbxMesh->GetLayer(0)->GetVertexColors();
                const bool hasTangents = fbxMesh->GetElementTangentCount() > 0 && fbxMesh->GetElementBinormalCount() > 0;
                if (hasColors) section.Colors.resize(vertexCount);
                if (recomputeTangents || hasTangents) section.Tangents.resize(vertexCount);

                // #pragma omp parallel for
                for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
                {
                    for (int vertIndexInFace = 0; vertIndexInFace < kPolygonCount; vertIndexInFace++)
                    {
                        const auto vertexIndex       = polyIndex * kPolygonCount + vertIndexInFace;
                        const auto controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, vertIndexInFace);

                        // position
                        section.Positions[vertexIndex] = ToVector3f(fbxMesh->GetControlPointAt(controlPointIndex));

                        // normal
                        FbxVector4 normal;
                        fbxMesh->GetPolygonVertexNormal(polyIndex, vertIndexInFace, normal);
                        section.Normals[vertexIndex] = ToVector3f(normal);

                        // tangent + bitangent sign (w)
                        if (!recomputeTangents && hasTangents)
                        {
                            const Vector3f T = ToVector3f(GetColorLayerElement(fbxMesh->GetElementTangent(0),  controlPointIndex, vertexIndex));
                            const Vector3f B = ToVector3f(GetColorLayerElement(fbxMesh->GetElementBinormal(0), controlPointIndex, vertexIndex));
                            const Vector3f N = section.Normals[vertexIndex];
                            // cross(N,T) 与 FBX 提供的 B 方向一致则 w=+1，否则 w=-1
                            const float w = Dot(Cross(N, T), B) > 0.0f ? 1.0f : -1.0f;
                            section.Tangents[vertexIndex] = Vector4f{T.x, T.y, T.z, w};
                        }

                        // UVs
                        for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                        {
                            auto uv = ToVector2f(GetUVLayerElement(fbxMesh->GetElementUV(uvIdx), controlPointIndex, vertexIndex));
                            if (inverseCoordsystem) uv.y = 1.0f - uv.y;
                            section.TexCoords[uvIdx][vertexIndex] = uv;
                        }

                        // color
                        if (hasColors)
                        {
                            section.Colors[vertexIndex] = ToColor(GetColorLayerElement(
                                fbxMesh->GetLayer(0)->GetVertexColors(), controlPointIndex, vertexIndex));
                        }

                        auto indicesValue = vertexIndex;
                        if (inverseCoordsystem)
                        {
                            if (vertIndexInFace == 1)
                                indicesValue += 1;
                            if (vertIndexInFace == 2)
                                indicesValue -= 1;
                        }
                        section.Indices[vertexIndex] = indicesValue;
                    }
                }

                if (recomputeTangents && numUV > 0)
                {
                    if (useMikktspace)
                        GenerateTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
                    else
                        GenerateSimpleTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
                }

                section.MaterialIndex = attrIndex;

                sections.push_back(std::move(section));
            }
        }
        if (sections.empty())
        {
            return nullptr;
        }
        return StaticMesh::StaticCreate(name, std::move(sections), std::move(materialNames));
    }
}
