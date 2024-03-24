#include "AssetSerializerUtil.h"


namespace pulsar
{

    ser::VarientRef AssetSerializerUtil::NewObject(const ser::VarientRef& ctx, Vector3f vec)
    {
        auto obj = ctx->New(ser::VarientType::Object);
        obj->Add("x", vec.x);
        obj->Add("y", vec.y);
        obj->Add("z", vec.z);
        return obj;
    }
    ser::VarientRef AssetSerializerUtil::NewObject(const ser::VarientRef& ctx, Vector4f vec)
    {
        auto obj = ctx->New(ser::VarientType::Object);
        obj->Add("x", vec.x);
        obj->Add("y", vec.y);
        obj->Add("z", vec.z);
        obj->Add("w", vec.w);
        return obj;
    }
    ser::VarientRef AssetSerializerUtil::NewObject(const ser::VarientRef& ctx, const Bounds3f& vec)
    {
        auto bound = ctx->New(ser::VarientType::Object);
        bound->Add("Extent", NewObject(ctx, vec.Extent));
        bound->Add("Origin", NewObject(ctx, vec.Origin));
        bound->Add("Sphere", vec.Sphere);
        return bound;
    }
    Vector3f AssetSerializerUtil::GetVector3Object(const ser::VarientRef& var)
    {
        float x = var->At("x")->AsFloat();
        float y = var->At("y")->AsFloat();
        float z = var->At("z")->AsFloat();
        return {x, y, z};
    }
    Vector4f AssetSerializerUtil::GetVector4Object(const ser::VarientRef& var)
    {
        float x = var->At("x")->AsFloat();
        float y = var->At("y")->AsFloat();
        float z = var->At("z")->AsFloat();
        float w = var->At("w")->AsFloat();
        return {x, y, z, w};
    }
    Bounds3f AssetSerializerUtil::GetBounds3Object(const ser::VarientRef& var)
    {
        auto extent = var->At("Extent");
        auto origin = var->At("Origin");
        auto sphere = var->At("Sphere")->AsFloat();

        Bounds3f bound;
        bound.Extent = GetVector3Object(extent);
        bound.Origin = GetVector3Object(origin);
        bound.Sphere = sphere;

        return bound;
    }
} // namespace pulsar