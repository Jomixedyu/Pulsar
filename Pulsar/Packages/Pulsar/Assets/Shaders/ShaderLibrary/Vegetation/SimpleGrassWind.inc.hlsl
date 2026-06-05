#ifndef _SIMPLEGRASSWIND_HLSL_
#define _SIMPLEGRASSWIND_HLSL_

#ifndef TWO_PI
#define TWO_PI      6.28318530717958647693
#endif

namespace vegetation
{
    namespace __detail__
    {
        half3 RotateAboutAxis(half4 NormalizedRotationAxisAndAngle, half3 PositionOnAxis, half3 Position)
        {
            // Project Position onto the rotation axis and find the closest point on the axis to Position
            half3 ClosestPointOnAxis = PositionOnAxis + NormalizedRotationAxisAndAngle.xyz * dot(
                NormalizedRotationAxisAndAngle.xyz, Position - PositionOnAxis);
            // Construct orthogonal axes in the plane of the rotation
            half3 UAxis = Position - ClosestPointOnAxis;
            half3 VAxis = cross(NormalizedRotationAxisAndAngle.xyz, UAxis);
            half CosAngle;
            half SinAngle;
            sincos(NormalizedRotationAxisAndAngle.w, SinAngle, CosAngle);
            // Rotate using the orthogonal axes
            half3 R = UAxis * CosAngle + VAxis * SinAngle;
            // Reconstruct the rotated world space position
            half3 RotatedPosition = ClosestPointOnAxis + R;
            // Convert from position to a position offset
            return RotatedPosition - Position;
        }
        half3 RotateAboutAxis(
            half3 NormalizedRotationAxis,
            half RotationAngle,
            half3 PivotPoint,
            half3 Position)
        {
            return RotateAboutAxis(half4(NormalizedRotationAxis, RotationAngle), PivotPoint, Position);
        }
    }

    
    /**
     * referenced from unreal engine
     * @param time 
     * @param windIntensity 
     * @param windHeight 
     * @param windSpeed 
     * @param additionalWPO 
     * @return 
     */
    half3 SimpleGrassWind(
        float time, half3 posWS,
        half windIntensity, half windHeight, half windSpeed, half3 additionalWPO)
    {
        half4 a = half4(0,
                        #ifdef Z_UP 
                        1, 0
                        #else
                        0, 1,
                        #endif
            1);
        half3 b = half3(0,
            #ifdef Z_UP
            0, 1
            #else
            1, 0
            #endif
            );

        half3 normedA = normalize(a.rgb);
        half3 normalizedRotationAxis = cross(normedA, b);
        
        float speed = time * windSpeed * -0.5f * a.a;

        float3 angleAPos = (normedA * speed) + (posWS / 10.24f) + 0.5f;
        half3 angleA = frac(angleAPos) * 2 + -1;
        angleA = abs(angleA);
        angleA = (3 - angleA * 2) * angleA * angleA;
        half angleAResult = dot(normedA, angleA);

        float3 angleBPos = posWS / 2.00f + speed + 0.5f;
        half3 angleB = frac(angleBPos) * 2 + -1;
        angleB = abs(angleB);
        angleB = (3 - angleB * 2) * angleB * angleB;
        half angleBResult = distance(angleB, 0);

        const half3 pivotOffset = half3(
        #ifdef Z_UP
        0, 0, -0.10
        #else
        0, -0.10, 0
        #endif
            );
        
        half rotationAngle = angleAResult + angleBResult;
        half3 pivotPoint = pivotOffset + additionalWPO;
        half3 position = additionalWPO;
        
        half3 result = __detail__::RotateAboutAxis(normalizedRotationAxis, rotationAngle * TWO_PI, pivotPoint, position);

        return result * windHeight * windIntensity + additionalWPO;
    }
}


#endif
