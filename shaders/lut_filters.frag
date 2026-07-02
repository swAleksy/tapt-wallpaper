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
} ubuf;

void main()
{
    vec4 baseColor = texture(sourceImage, qt_TexCoord0);

    float blueColor = baseColor.b * (ubuf.lutSize - 1.0);

    vec2 quad1;
    quad1.y = floor(floor(blueColor) / ubuf.lutSize);
    quad1.x = floor(blueColor) - (quad1.y * ubuf.lutSize);

    vec2 quad2;
    quad2.y = floor(ceil(blueColor) / ubuf.lutSize);
    quad2.x = ceil(blueColor) - (quad2.y * ubuf.lutSize);

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
