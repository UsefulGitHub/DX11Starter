#ifndef __GGP_NOISE_HEADER__
#define __GGP_NOISE_HEADER__

// Simplex 2D noise
// Adapted from "GLSL Noise Algorithms" by patriciogonzalezvivo on GitHub
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

float3 permute(float3 x) { return fmod(((x * 34.0) + 1.0) * x, 289.0); }

float snoise(float2 v) {
    const float4 C = float4(0.211324865405187, 0.366025403784439,
        -0.577350269189626, 0.024390243902439);
    float2 i = floor(v + dot(v, C.yy));
    float2 x0 = v - i + dot(i, C.xx);
    float2 i1;
    i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    float4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = fmod(i, 289.0);
    float3 p = permute(permute(i.y + float3(0.0, i1.y, 1.0))
        + i.x + float3(0.0, i1.x, 1.0));
    float3 m = max(0.5 - float3(dot(x0, x0), dot(x12.xy, x12.xy),
        dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    float3 x = 2.0 * frac(p * C.www) - 1.0;
    float3 h = abs(x) - 0.5;
    float3 ox = floor(x + 0.5);
    float3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    float3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

// Simplex 3D Noise 
// by Ian McEwan, Ashima Arts
// Adapted from "GLSL Noise Algorithms" by patriciogonzalezvivo on GitHub
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

float4 permute(float4 x) { return fmod(((x * 34.0) + 1.0) * x, 289.0); }
float4 taylorInvSqrt(float4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(float3 v) {
    const float2  C = float2(1.0 / 6.0, 1.0 / 3.0);
    const float4  D = float4(0.0, 0.5, 1.0, 2.0);

    // First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

    //  x0 = x0 - 0. + 0.0 * C 
    float3 x1 = x0 - i1 + 1.0 * C.xxx;
    float3 x2 = x0 - i2 + 2.0 * C.xxx;
    float3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = fmod(i, 289.0);
    float4 p = permute(permute(permute(
        i.z + float4(0.0, i1.z, i2.z, 1.0))
        + i.y + float4(0.0, i1.y, i2.y, 1.0))
        + i.x + float4(0.0, i1.x, i2.x, 1.0));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0 / 7.0; // N=7
    float3  ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  fmod(p,N*N)

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_);    // fmod(j,N)

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0.0, 0.0, 0.0, 0.0));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

    //Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // lerp final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1),
        dot(p2, x2), dot(p3, x3)));
}

float4 fade(float4 t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }
// Classic Perlin noise, periodic version
float cnoise(float4 P, float4 rep) {
    float4 Pi0 = fmod(floor(P), rep); // Integer part modulo rep
    float4 Pi1 = fmod(Pi0 + 1.0, rep); // Integer part + 1 fmod rep
    float4 Pf0 = frac(P); // Fractional part for interpolation
    float4 Pf1 = Pf0 - 1.0; // Fractional part - 1.0
    float4 ix = float4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    float4 iy = float4(Pi0.yy, Pi1.yy);
    float4 iz0 = float4(Pi0.zzzz);
    float4 iz1 = float4(Pi1.zzzz);
    float4 iw0 = float4(Pi0.wwww);
    float4 iw1 = float4(Pi1.wwww);

    float4 ixy = permute(permute(ix) + iy);
    float4 ixy0 = permute(ixy + iz0);
    float4 ixy1 = permute(ixy + iz1);
    float4 ixy00 = permute(ixy0 + iw0);
    float4 ixy01 = permute(ixy0 + iw1);
    float4 ixy10 = permute(ixy1 + iw0);
    float4 ixy11 = permute(ixy1 + iw1);

    float4 gx00 = ixy00 / 7.0;
    float4 gy00 = floor(gx00) / 7.0;
    float4 gz00 = floor(gy00) / 6.0;
    gx00 = frac(gx00) - 0.5;
    gy00 = frac(gy00) - 0.5;
    gz00 = frac(gz00) - 0.5;
    float4 gw00 = float4(0.75, 0.75, 0.75, 0.75) - abs(gx00) - abs(gy00) - abs(gz00);
    float4 sw00 = step(gw00, float4(0.0, 0.0, 0.0, 0.0));
    gx00 -= sw00 * (step(0.0, gx00) - 0.5);
    gy00 -= sw00 * (step(0.0, gy00) - 0.5);

    float4 gx01 = ixy01 / 7.0;
    float4 gy01 = floor(gx01) / 7.0;
    float4 gz01 = floor(gy01) / 6.0;
    gx01 = frac(gx01) - 0.5;
    gy01 = frac(gy01) - 0.5;
    gz01 = frac(gz01) - 0.5;
    float4 gw01 = float4(0.75, 0.75, 0.75, 0.75) - abs(gx01) - abs(gy01) - abs(gz01);
    float4 sw01 = step(gw01, float4(0.0, 0.0, 0.0, 0.0));
    gx01 -= sw01 * (step(0.0, gx01) - 0.5);
    gy01 -= sw01 * (step(0.0, gy01) - 0.5);

    float4 gx10 = ixy10 / 7.0;
    float4 gy10 = floor(gx10) / 7.0;
    float4 gz10 = floor(gy10) / 6.0;
    gx10 = frac(gx10) - 0.5;
    gy10 = frac(gy10) - 0.5;
    gz10 = frac(gz10) - 0.5;
    float4 gw10 = float4(0.75, 0.75, 0.75, 0.75) - abs(gx10) - abs(gy10) - abs(gz10);
    float4 sw10 = step(gw10, float4(0.0, 0.0, 0.0, 0.0));
    gx10 -= sw10 * (step(0.0, gx10) - 0.5);
    gy10 -= sw10 * (step(0.0, gy10) - 0.5);

    float4 gx11 = ixy11 / 7.0;
    float4 gy11 = floor(gx11) / 7.0;
    float4 gz11 = floor(gy11) / 6.0;
    gx11 = frac(gx11) - 0.5;
    gy11 = frac(gy11) - 0.5;
    gz11 = frac(gz11) - 0.5;
    float4 gw11 = float4(0.75, 0.75, 0.75, 0.75) - abs(gx11) - abs(gy11) - abs(gz11);
    float4 sw11 = step(gw11, float4(0.0, 0.0, 0.0, 0.0));
    gx11 -= sw11 * (step(0.0, gx11) - 0.5);
    gy11 -= sw11 * (step(0.0, gy11) - 0.5);

    float4 g0000 = float4(gx00.x, gy00.x, gz00.x, gw00.x);
    float4 g1000 = float4(gx00.y, gy00.y, gz00.y, gw00.y);
    float4 g0100 = float4(gx00.z, gy00.z, gz00.z, gw00.z);
    float4 g1100 = float4(gx00.w, gy00.w, gz00.w, gw00.w);
    float4 g0010 = float4(gx10.x, gy10.x, gz10.x, gw10.x);
    float4 g1010 = float4(gx10.y, gy10.y, gz10.y, gw10.y);
    float4 g0110 = float4(gx10.z, gy10.z, gz10.z, gw10.z);
    float4 g1110 = float4(gx10.w, gy10.w, gz10.w, gw10.w);
    float4 g0001 = float4(gx01.x, gy01.x, gz01.x, gw01.x);
    float4 g1001 = float4(gx01.y, gy01.y, gz01.y, gw01.y);
    float4 g0101 = float4(gx01.z, gy01.z, gz01.z, gw01.z);
    float4 g1101 = float4(gx01.w, gy01.w, gz01.w, gw01.w);
    float4 g0011 = float4(gx11.x, gy11.x, gz11.x, gw11.x);
    float4 g1011 = float4(gx11.y, gy11.y, gz11.y, gw11.y);
    float4 g0111 = float4(gx11.z, gy11.z, gz11.z, gw11.z);
    float4 g1111 = float4(gx11.w, gy11.w, gz11.w, gw11.w);

    float4 norm00 = taylorInvSqrt(float4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)));
    g0000 *= norm00.x;
    g0100 *= norm00.y;
    g1000 *= norm00.z;
    g1100 *= norm00.w;

    float4 norm01 = taylorInvSqrt(float4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)));
    g0001 *= norm01.x;
    g0101 *= norm01.y;
    g1001 *= norm01.z;
    g1101 *= norm01.w;

    float4 norm10 = taylorInvSqrt(float4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)));
    g0010 *= norm10.x;
    g0110 *= norm10.y;
    g1010 *= norm10.z;
    g1110 *= norm10.w;

    float4 norm11 = taylorInvSqrt(float4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)));
    g0011 *= norm11.x;
    g0111 *= norm11.y;
    g1011 *= norm11.z;
    g1111 *= norm11.w;

    float n0000 = dot(g0000, Pf0);
    float n1000 = dot(g1000, float4(Pf1.x, Pf0.yzw));
    float n0100 = dot(g0100, float4(Pf0.x, Pf1.y, Pf0.zw));
    float n1100 = dot(g1100, float4(Pf1.xy, Pf0.zw));
    float n0010 = dot(g0010, float4(Pf0.xy, Pf1.z, Pf0.w));
    float n1010 = dot(g1010, float4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
    float n0110 = dot(g0110, float4(Pf0.x, Pf1.yz, Pf0.w));
    float n1110 = dot(g1110, float4(Pf1.xyz, Pf0.w));
    float n0001 = dot(g0001, float4(Pf0.xyz, Pf1.w));
    float n1001 = dot(g1001, float4(Pf1.x, Pf0.yz, Pf1.w));
    float n0101 = dot(g0101, float4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
    float n1101 = dot(g1101, float4(Pf1.xy, Pf0.z, Pf1.w));
    float n0011 = dot(g0011, float4(Pf0.xy, Pf1.zw));
    float n1011 = dot(g1011, float4(Pf1.x, Pf0.y, Pf1.zw));
    float n0111 = dot(g0111, float4(Pf0.x, Pf1.yzw));
    float n1111 = dot(g1111, Pf1);

    float4 fade_xyzw = fade(Pf0);
    float4 n_0w = lerp(float4(n0000, n1000, n0100, n1100), float4(n0001, n1001, n0101, n1101), fade_xyzw.w);
    float4 n_1w = lerp(float4(n0010, n1010, n0110, n1110), float4(n0011, n1011, n0111, n1111), fade_xyzw.w);
    float4 n_zw = lerp(n_0w, n_1w, fade_xyzw.z);
    float2 n_yzw = lerp(n_zw.xy, n_zw.zw, fade_xyzw.y);
    float n_xyzw = lerp(n_yzw.x, n_yzw.y, fade_xyzw.x);
    return 2.2 * n_xyzw;
}

float fbm(float3 p)
{
    float f;
    f = 0.50000 * (snoise(p)); p = p * 2.01;
    f += 0.25000 * (snoise(p)); p = p * 2.02;
    f += 0.12500 * (snoise(p)); p = p * 2.03;
    f += 0.06250 * (snoise(p)); p = p * 2.04;
    f += 0.03125 * (snoise(p)); p = p * 2.05;
    f += 0.015625 * (snoise(p));
    return f;
}

float fbm(float2 p)
{
    float f;
    f = 0.50000 * (snoise(p)); p = p * 2.01;
    f += 0.25000 * (snoise(p)); p = p * 2.02;
    f += 0.12500 * (snoise(p)); p = p * 2.03;
    f += 0.06250 * (snoise(p)); p = p * 2.04;
    f += 0.03125 * (snoise(p)); p = p * 2.05;
    f += 0.015625 * (snoise(p));
    return f;
}

float fbmPerlin(float4 p, float4 rep)
{
    float f;
    f = 0.50000 * (cnoise(p, rep)); p = p * 2.01;
    f += 0.25000 * (cnoise(p, rep)); p = p * 2.02;
    f += 0.12500 * (cnoise(p, rep)); p = p * 2.03;
    f += 0.06250 * (cnoise(p, rep)); p = p * 2.04;
    f += 0.03125 * (cnoise(p, rep)); p = p * 2.05;
    f += 0.015625 * (cnoise(p, rep));
    return f;
}

// Samples three dimensional noise with points along the circumfrence a two-dimensional circle
// This means the noise never tiles or shows seams
// Adapted from Toni Sagrista's blog about Gaia Sky
// https://tonisagrista.com/blog/2021/procedural-planetary-surfaces/#seamless-tilable-noise

float spinny(float phi, float theta) {
    return fbm(float3(
        cos(phi) * cos(theta),
        cos(phi) * sin(theta),
        sin(phi)
        )
    );
}

#endif