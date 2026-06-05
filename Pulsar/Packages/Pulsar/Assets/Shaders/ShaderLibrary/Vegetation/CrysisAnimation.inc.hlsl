
namespace vegetation
{
    struct CrysisAnimationParameter
    {
        half BendScale;
        half3 NormalWS;
        half3 PositionWS;
        half2 WindDir;

        float Time;
        
        half BranchPhase;
        half BranchAmp;
        half BranchAtten;
        
        half DetailPhase;
        half DetailAmp;
        half DetailEdgeAtten;
        
        half Freq;
    };
    
    #pragma region CrysisAnimation
    namespace __detail__
    {
        half4 SmoothCurve(half4 x) { return x * x * (3.0 - 2.0 * x); }
        half4 TriangleWave(float4 x) { return abs(frac(x + 0.5) * 2.0 - 1.0); }
        half4 SmoothTriangleWave(float4 x) { return SmoothCurve(TriangleWave(x)); }

        half3 WindTrunkBending( half3 vPos, half2 vWind, half fBendFactor )
        {
            // Smooth the bending factor and increase 
            // the near by height limit.
            fBendFactor += 1.0;
            fBendFactor *= fBendFactor;
            fBendFactor = fBendFactor * fBendFactor - fBendFactor;

            // Displace the vert.
            half3 vNewPos = vPos;
            vNewPos.xz += vWind * fBendFactor;


            // Limit the length which makes the bend more 
            // spherical and prevents stretching.
            half fLength = length( vPos );
            vPos = normalize( vNewPos ) * fLength;

            return vPos;
        }

        half3 WindBranchBending(  half3 vPos,
                                   half3 vNormal,

                                   float fTime, 
                                   half fWindSpeed,

                                   half fBranchPhase,
                                   half fBranchAmp,
                                   half fBranchAtten,

                                   half fDetailPhase,
                                   half fDetailAmp,
                                   half fDetailEdgeAtten,
                                   
                                   half fFreq)
        {
            half fVertPhase = dot( vPos, fDetailPhase + fBranchPhase );

            float2 vWavesIn = fTime + half2( fVertPhase, fBranchPhase );

            float4 vWaves = ( frac( vWavesIn.xxyy *
                                    half4( 1.975, 0.793, 0.375, 0.193 ) ) *
                                    2.0 - 1.0 ) * fWindSpeed * fFreq;

            vWaves = SmoothTriangleWave( vWaves );

            half2 vWavesSum = vWaves.xz + vWaves.yw;

            // We want the branches to bend both up and down.
            // vWavesSum.y = 1 - ( vWavesSum.y * 2 );

            // vPos += vWavesSum.xyx * half3(  fEdgeAtten * fDetailAmp * vNormal.x,
            //                                 fBranchAtten * fBranchAmp,
            //                                 fEdgeAtten * fDetailAmp * vNormal.y );
            vPos.xyz += vWavesSum.x * half3(fDetailEdgeAtten * fDetailAmp * vNormal.xyz);
            vPos.y += vWavesSum.y * fBranchAtten * fBranchAmp;
            
            return vPos;
        }
    }
    #pragma endregion
                
    half3 CrysisAnimation(CrysisAnimationParameter input)
    {
        half3 pos = __detail__::WindTrunkBending(input.PositionWS, input.WindDir, input.BendScale);
        pos = __detail__::WindBranchBending(pos, input.NormalWS, input.Time, 1,
            input.BranchPhase, input.BranchAmp, input.BranchAtten,
            input.DetailPhase, input.DetailAmp, input.DetailEdgeAtten,
            input.Freq
            );
        
        return pos;
    }

    struct FoliageAnimMaskColor
    {
        half BendScale;
        half OverallStiffness;
        half LeafEdgeStiffness;
        half LeafPhase;
    };
    FoliageAnimMaskColor GetDefaultLayoutFoliageAnimMaskColor(half4 color)
    {
        FoliageAnimMaskColor c = (FoliageAnimMaskColor)0;
        c.BendScale = color.a;
        c.OverallStiffness = 1 - color.b;
        c.LeafEdgeStiffness = color.r;
        c.LeafPhase = color.g;
        return c;
    }
}