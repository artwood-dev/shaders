#version 330 core
in vec2 UV;              // 0..1 across the window
out vec4 FragColor;
uniform float iTime;     // seconds since start
uniform float iAspect;   // width / height

float SDF(vec3 p) {
    float sphere = length(p) - 1;
    float ground = p.y + 1;
    return min(sphere,ground); //Sphere of Radius 1
}

float Raymarching(vec3 ray_origin, vec3 ray_direction) {
    float t = 0;
    for (int i = 0; i < 80; i++) {
        vec3 p = ray_origin + ray_direction*t;

        float d = SDF(p);

        t += d;

        if (d<0.001) break;
        if (t > 100) {
            t=100;
            break;
        }
    }
    return t;
}

void main() {
    vec2 screen = ((UV - 0.5) * 2) * vec2(iAspect, 1);

    //Ray Origin and Direction
    vec3 ro = vec3(0, 0, -3);
    vec3 rd = normalize( vec3(screen, 1) );

    float r1 = Raymarching ( ro, rd );
    float r2 = Raymarching ( ro + vec3(0.001,0,0), rd );
    float r3 = Raymarching ( ro + vec3(0,0.001,0), rd );
    float r4 = Raymarching ( ro + vec3(0,0,0.001), rd );

    vec3 norm = vec3(r2-r1, r3-r1, r4-r1);

    vec3 col = vec3(1,1,1);
    vec3 light = vec3(sin(iTime), 1, cos(iTime)) * 300;
    //vec3 light = vec3(1,1,1) * 100;

    FragColor = vec4(col * dot(norm,light), 1.0);
    //FragColor = vec4(norm*100, 0);
}
