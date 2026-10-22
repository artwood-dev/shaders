#version 330 core
in vec2 UV;              // 0..1 across the window
out vec4 FragColor;
uniform float iTime;     // seconds since start
uniform float iAspect;   // width / height

void main() {
    FragColor = vec4(UV, 0.0, 1.0);
}
