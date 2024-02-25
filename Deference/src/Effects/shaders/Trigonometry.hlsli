#ifndef __TRIGONOMETRY_HLSLI__
#define __TRIGONOMETRY_HLSLI__

#define M_PI  3.14159265358979323846264338327950288
#define M_1_PI  0.318309886183790671538

float atan2_WAR(float y, float x)
{
    if (x > 0.f)
        return atan(y / x);
    else if (x < 0.f && y >= 0.f)
        return atan(y / x) + M_PI;
    else if (x < 0.f && y < 0.f)
        return atan(y / x) - M_PI;
    else if (x == 0.f && y > 0.f)
        return M_PI / 2.f;
    else if (x == 0.f && y < 0.f)
        return -M_PI / 2.f;
    return 0.f; // x==0 && y==0 (undefined)
}

float2 worldDirToPolorCoords(float3 dir)
{
    float3 p = normalize(dir);

	// atan2_WAR is a work-around due to an apparent compiler bug in atan2
    float u = (1.f + atan2_WAR(p.x, -p.z) * M_1_PI) * 0.5f;
    float v = acos(p.y) * M_1_PI;
    return float2(u, v);
}

#endif