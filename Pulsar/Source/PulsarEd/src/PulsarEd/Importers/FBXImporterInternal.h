#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Assets/SkinnedMesh.h>
#include <Pulsar/Assets/Skeleton.h>
#include <CoreLib/UString.h>
#include <fbxsdk.h>

namespace pulsared
{
    // -----------------------------------------------------------------------
    // 类型转换
    // -----------------------------------------------------------------------
    inline Vector3f ToVector3f(const FbxVector4& vec)
    {
        return {static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2])};
    }
    inline Vector3f ToVector3f(const FbxDouble3& vec)
    {
        return {static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2])};
    }
    inline Vector2f ToVector2f(const FbxVector2& vec)
    {
        return {(float)vec[0], (float)vec[1]};
    }
    inline Color4b ToColor(const FbxColor& color)
    {
        return Color4b{uint8_t(color.mRed * 255), uint8_t(color.mGreen * 255), uint8_t(color.mBlue * 255), uint8_t(color.mAlpha * 255) };
    }
    inline Quat4f ToQuat(const FbxQuaternion& q)
    {
        return {(float)q[0], (float)q[1], (float)q[2], (float)q[3]};
    }

    // -----------------------------------------------------------------------
    // LayerElement 读取
    // -----------------------------------------------------------------------
    template <typename T>
    T GetColorLayerElement(FbxLayerElementTemplate<T>* layer, int ctrlPoint, int vertIndex)
    {
        const auto mappingMode   = layer->GetMappingMode();
        const auto referenceMode = layer->GetReferenceMode();
        switch (mappingMode)
        {
        case FbxGeometryElement::EMappingMode::eByControlPoint: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(ctrlPoint);
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(ctrlPoint);
                return layer->GetDirectArray().GetAt(index);
            }
            default:
                assert(false);
                break;
            }
            break;
        }
        case FbxGeometryElement::EMappingMode::eByPolygonVertex: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(vertIndex);
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(vertIndex);
                return layer->GetDirectArray().GetAt(index);
            }
            default:
                assert(false);
                break;
            }
            break;
        }
        default:
            assert(false);
        }
        return {};
    }

    template <typename T>
    T GetUVLayerElement(FbxLayerElementTemplate<T>* layer, int ctrlPoint, int vertIndex)
    {
        const auto mappingMode   = layer->GetMappingMode();
        const auto referenceMode = layer->GetReferenceMode();
        switch (mappingMode)
        {
        case FbxGeometryElement::EMappingMode::eByControlPoint: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(ctrlPoint);
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(ctrlPoint);
                return layer->GetDirectArray().GetAt(index);
            }
            default:
                assert(false);
                break;
            }
            break;
        }
        case FbxGeometryElement::EMappingMode::eByPolygonVertex: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(vertIndex);
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(vertIndex);
                return layer->GetDirectArray().GetAt(index);
            }
            default:
                assert(false);
                break;
            }
            break;
        }
        default:
            assert(false);
        }
        return {};
    }

    // -----------------------------------------------------------------------
    // 切线生成
    // -----------------------------------------------------------------------
    void GenerateTangentsForSection(
        const array_list<Vector3f>& positions,
        const array_list<Vector3f>& normals,
        const array_list<Vector2f>& uvs,
        array_list<Vector4f>& outTangents);

    void GenerateSimpleTangentsForSection(
        const array_list<Vector3f>& positions,
        const array_list<Vector3f>& normals,
        const array_list<Vector2f>& uvs,
        array_list<Vector4f>& outTangents);

    // -----------------------------------------------------------------------
    // Mesh / Skeleton 处理函数
    // -----------------------------------------------------------------------
    RCPtr<StaticMesh> ProcessMesh(FbxNode* fbxNode, bool inverseCoordsystem, bool recomputeTangents, bool useMikktspace);
    bool HasSkinDeformer(FbxMesh* fbxMesh);
    RCPtr<SkinnedMesh> ProcessSkinnedMesh(FbxNode* fbxNode, RCPtr<Skeleton> skeleton, bool inverseCoordsystem, bool recomputeTangents, bool useMikktspace);
    RCPtr<Skeleton> ProcessSkeleton(FbxScene* fbxScene, const string& name, bool inverseCoordSystem);
}
