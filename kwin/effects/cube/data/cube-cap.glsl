uniform sampler2D sample;
uniform vec4 u_capColor;
uniform float u_opacity;
uniform int u_mirror;

varying vec2 varyingTexCoords;

vec2 mirrorTex(vec2 coords) {
    vec2 mirrored = coords;
    if (u_mirror != 0) {
        mirrored.t = mirrored.t * (-1.0) + 1.0;
    }
    return mirrored;
}

void main() {
    vec4 color = u_capColor;
    vec2 texCoord = mirrorTex(varyingTexCoords);
    vec4 tex = texture2D(sample, texCoord);
    if (texCoord.s < 0.0 || texCoord.s > 1.0 ||
            texCoord.t < 0.0 || texCoord.t > 1.0) {
        tex = u_capColor;
    }
    color.rgb = tex.rgb*tex.a + color.rgb*(1.0-tex.a);
    color.a = u_opacity;

    gl_FragColor = color;
}