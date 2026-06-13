// Fragment snippet du nuage de points (CustomMaterial Unshaded, blend additif).
// Masque rond + dégradé cyan↔magenta selon le bruit + blanchiment sur charge aiguë.
// Uniforms : uColA, uColB (couleurs accent), uHigh.

VARYING float vNoise;
VARYING vec2 vCorner;

void MAIN()
{
    float r2 = dot(vCorner, vCorner);
    if (r2 > 1.0)
        discard; // hors du disque inscrit dans le quad → transparent

    vec3 col = mix(uColA.rgb, uColB.rgb, vNoise * 0.5 + 0.5);
    col += vec3(1.0) * uHigh * 0.22; // pics chauds → blanchiment

    // Bord doux (antialiasing du point, glow vers l'extérieur).
    float edge = smoothstep(1.0, 0.2, r2);

    // Unshaded : FRAGCOLOR = couleur finale, a = opacité (modulée par le blend additif).
    // Alpha modéré : en blend additif, des points qui se chevauchent saturent vite en blanc.
    FRAGCOLOR = vec4(col, 0.55 * edge);
}
