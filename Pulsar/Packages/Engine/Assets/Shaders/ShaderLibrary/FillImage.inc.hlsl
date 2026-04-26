


// Stretch 拉伸填满，会变形
float2 FillImage_Stretch(float2 screenUV, float2 screenSize, float2 texSize)
{
    return screenUV;
}

// Cover 模式：填满屏幕，多余部分裁剪，纹理不变形
float2 FillImage_Cover(float2 screenUV, float2 screenSize, float2 texSize, float2 pivot = 0.5)
{
    float screenAspect = screenSize.x / screenSize.y;
    float texAspect = texSize.x / texSize.y;
    float ratio = screenAspect / texAspect;

    float2 uv = screenUV - pivot;

    if (ratio > 1.0)
    {
        uv.y /= ratio;
    }
    else
    {
        uv.x *= ratio;
    }

    return uv + pivot;
}

// Contain 模式：完整显示纹理，不裁剪，空余区域超出 [0,1]，纹理不变形
float2 FillImage_Contain(float2 screenUV, float2 screenSize, float2 texSize, float2 pivot = 0.5)
{
    float screenAspect = screenSize.x / screenSize.y;
    float texAspect = texSize.x / texSize.y;
    float ratio = screenAspect / texAspect;

    float2 uv = screenUV - pivot;

    if (ratio > 1.0)
    {
        uv.x /= ratio;
    }
    else
    {
        uv.y *= ratio;
    }

    return uv + pivot;
}

// ScaleDown - 如果纹理比屏幕大则 Contain，否则原始大小居中
float2 FillImage_ScaleDown(float2 screenUV, float2 screenSize, float2 texSize, float2 pivot = 0.5)
{
    if (texSize.x <= screenSize.x && texSize.y <= screenSize.y)
    {
        float2 uv = (screenUV -pivot) * screenSize / texSize;
        return uv + pivot;
    }
    return FillImage_Contain(screenUV, screenSize, texSize, pivot);
}

// Tile (Repeat) - 原始大小平铺
float2 FillImage_Tile(float2 screenUV, float2 screenSize, float2 texSize)
{
    return screenUV * screenSize / texSize;
}
