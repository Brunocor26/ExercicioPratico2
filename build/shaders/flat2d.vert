#version 330 core
layout(location=0) in vec2 aPos;   // posição 2D (em coordenadas "de caderno")
uniform vec4 uBounds;              // (minX, minY, maxX, maxY)

void main() {
    float minX=uBounds.x, minY=uBounds.y, maxX=uBounds.z, maxY=uBounds.w;
    vec2 ndc;
    ndc.x = ((aPos.x - minX) / (maxX - minX)) * 2.0 - 1.0;
    ndc.y = ((aPos.y - minY) / (maxY - minY)) * 2.0 - 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
}