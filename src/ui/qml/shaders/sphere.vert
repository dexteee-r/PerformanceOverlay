// Vertex snippet du nuage de points « FLUX DE CHARGE » (CustomMaterial Qt Quick 3D).
// Chaque point arrive en 4 sommets (un quad) : VERTEX = position unitaire sur la
// sphère (sert aussi de normale, sphère centrée à l'origine), UV0 = coin du quad
// dans [-1,1]. On déplace le centre du point le long de sa normale par du bruit
// simplex 3D, puis on billboarde le quad face caméra en espace vue → POINT_SIZE
// contrôlé en unités monde (rétrécit naturellement avec la distance).
//
// Uniforms (= propriétés de la CustomMaterial) :
//   uTime, uMid (CPU 0..1), uBass (GPU 0..1), uHigh (temp+réseau 0..1), uPointSize.

VARYING float vNoise;
VARYING vec2 vCorner;

// ---- Bruit simplex 3D (Ashima Arts / Stefan Gustavson, domaine public) ----
vec3 sp_mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 sp_mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 sp_permute(vec4 x) { return sp_mod289(((x * 34.0) + 1.0) * x); }
vec4 sp_taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
float snoise(vec3 v)
{
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;
    i = sp_mod289(i);
    vec4 p = sp_permute(sp_permute(sp_permute(
                 i.z + vec4(0.0, i1.z, i2.z, 1.0))
               + i.y + vec4(0.0, i1.y, i2.y, 1.0))
               + i.x + vec4(0.0, i1.x, i2.x, 1.0));
    float n_ = 0.142857142857;
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);
    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);
    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);
    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;
    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);
    vec4 norm = sp_taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

void MAIN()
{
    vec3 center = VERTEX; // point unitaire == normale

    float speed = uTime * (0.18 + uMid * 0.5);
    float n = snoise(center * 1.5 + vec3(speed));
    float spikes = snoise(center * 5.0 + vec3(uTime)) * uHigh * 0.14;

    // Déplacement le long de la normale : pos + normal*d == center*(1+d) (|center|=1).
    // uBass/uHigh peuvent dépasser 1 (jusqu'à ~1.5 quand on crie) → vagues qui
    // s'allongent au lieu de plafonner. 0.24 = longueur max des vagues (monter pour
    // des ondulations plus amples).
    float disp = n * (0.05 + uBass * 0.24) + spikes;
    vec3 displaced = center * (1.0 + disp);

    // Centre en espace vue, puis offset du coin du quad (billboard).
    vec4 viewPos = VIEW_MATRIX * MODEL_MATRIX * vec4(displaced, 1.0);
    float sz = uPointSize * (1.0 + uBass * 0.5);
    viewPos.xy += UV0 * sz;
    POSITION = PROJECTION_MATRIX * viewPos;

    vNoise = n;
    vCorner = UV0;
}
