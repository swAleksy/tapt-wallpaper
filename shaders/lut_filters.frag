#version 450

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D sourceImage;
layout(binding = 2) uniform sampler2D lutTexture;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float lutSize;
    float filterMix;
    float hue; // NOWE: przesunięcie barwy, zakres -1.0 … +1.0 (odpowiada -180° … +180°)
} ubuf;

// Standardowa konwersja RGB -> HSV
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// Odwrotność powyższej konwersji: HSV -> RGB
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 baseColor = texture(sourceImage, qt_TexCoord0);
    //float ubuf.lutSize = float(textureSize(lutTexture, 0).y);

    // NOWE: obrót barwy — ta sama konwencja co w LutService::processWallpaperOpenCV
    // (tam hue*180° w przestrzeni HSV; tutaj hue jest znormalizowane do pełnego koła = 1.0,
    // więc przesunięcie o 180° odpowiada 0.5)
    if (ubuf.hue != 0.0) {
        vec3 hsv = rgb2hsv(baseColor.rgb);
        hsv.x = fract(hsv.x + ubuf.hue * 0.5);
        baseColor.rgb = hsv2rgb(hsv);
    }

    float blueColor = baseColor.b * (ubuf.lutSize - 1.0);

    vec2 quad1;
    quad1.y = floor(floor(blueColor) / ubuf.lutSize);
    quad1.x = floor(blueColor) - (quad1.y * ubuf.lutSize);

    vec2 quad2;
    quad2.y = floor(ceil(blueColor) / ubuf.lutSize);
    quad2.x = min(ceil(blueColor), ubuf.lutSize - 1.0) - (quad2.y * ubuf.lutSize);

    vec2 texPos1;
    texPos1.x = (quad1.x * ubuf.lutSize) + 0.5 + ((ubuf.lutSize - 1.0) * baseColor.r);
    texPos1.y = (quad1.y * ubuf.lutSize) + 0.5 + ((ubuf.lutSize - 1.0) * baseColor.g);
    texPos1.x /= (ubuf.lutSize * ubuf.lutSize);
    texPos1.y /= ubuf.lutSize;

    vec2 texPos2;
    texPos2.x = (quad2.x * ubuf.lutSize) + 0.5 + ((ubuf.lutSize - 1.0) * baseColor.r);
    texPos2.y = (quad2.y * ubuf.lutSize) + 0.5 + ((ubuf.lutSize - 1.0) * baseColor.g);
    texPos2.x /= (ubuf.lutSize * ubuf.lutSize);
    texPos2.y /= ubuf.lutSize;

    vec4 newColor1 = texture(lutTexture, texPos1);
    vec4 newColor2 = texture(lutTexture, texPos2);

    vec4 finalColor = mix(newColor1, newColor2, fract(blueColor));

    outColor = mix(baseColor, vec4(finalColor.rgb, baseColor.a), ubuf.filterMix) * ubuf.qt_Opacity;
}
