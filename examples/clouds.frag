#version 330 core
in vec2 UV;              // 0..1 across the window
out vec4 FragColor;
uniform float iTime;     // seconds since start
uniform float iAspect;   // width / height

float whiteNoise(vec2 coord) {
    uvec2 p = uvec2(ivec2(coord));
    uint state = p.x * 19349669u + p.y * 83492837u;
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state)
                * 277803737u;
    word = (word >> 22u) ^ word;
    return float(word) / 4294967296.0;
}

float smoothNoise(vec2 coord, float scale) {
    vec2 p = coord * scale;
    vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 u = f * f * (3.0 - 2.0 * f);

    float a = whiteNoise(i);
    float b = whiteNoise(i + vec2(1.0, 0.0));
    float c = whiteNoise(i + vec2(0.0, 1.0));
    float d = whiteNoise(i + vec2(1.0, 1.0));

    return mix(
        mix(a, b, u.x),
        mix(c, d, u.x),
        u.y
    );
}

float fractalNoise(vec2 coord, float scale, int depth) {
    float sum = 0;
    float amplitude = 0.5;
    float frequency = 1;

    for (int i = 0; i < depth; i++) {
        sum += smoothNoise(coord, scale*frequency) * amplitude;
        amplitude *= 0.5;
        frequency *= 2;
    }

    return sum;
}

float dist2Ground (float altitude, vec3 viewDir) {
    if(viewDir.z < 0) {
        return altitude / -viewDir.z;
    }
    else return mix(1000, 0, viewDir.z * 2.5);
}

vec2 projectedCoords (float altitude, vec3 viewDir) {
    //camera ray intersection
    //camera position + t*camera direction = 0
    //cameraPosition.z = -t*cameraDirection.z
    //t = altitude/viewDir.z
    //Projected Coord = camPos + t*viewDir

    float t = dist2Ground(altitude, viewDir);
    vec3 camPos = vec3(0,0,altitude);
    vec3 result3 = camPos + viewDir*t;
    return result3.xy;
}

void main() {
    float altitude = 10;
    float camAngle=1;
    vec2 noiseOffset = vec2(-iTime*2,0);

    vec2 screen = ((UV - 0.5) * 2) * vec2(iAspect, 1);

    vec3 viewDir = vec3(1,0,0) * screen.x + 
        vec3(0, cos(camAngle), sin(camAngle)) * screen.y + 
        vec3(0, sin(camAngle), -cos(camAngle));

    vec2 uv_proj = projectedCoords (altitude, viewDir) + noiseOffset;

    float noise = fractalNoise(uv_proj, 0.04, 8);
    float land = float(noise > 0.5);
    float coast = land - float(noise>0.51);
    float ocean = noise * (1-land);
    float waves = (float(noise < 0.495) * float(noise > 0.493))*0.6 + 
            (float(noise < 0.49) * float(noise > 0.488))*0.3 + 
            (float(noise < 0.485) * float(noise > 0.482))*0.1;

    vec3 oceanColor = vec3(0.0352, 0.7647, 0.8588) * 1.4;
    vec3 landColor = vec3(0.2039, 0.5490, 0.1921) * 2.5;
    vec3 coastColor = vec3(0.7607, 0.6980, 0.5019) * 1.3;

    vec3 col = vec3(0,0,0);
    col = mix(oceanColor*ocean,vec3(1.0,1.0,1.0),waves);
    col = mix(col,landColor * noise,land);
    col = mix(col,coastColor,coast);

    vec3 extinction = vec3(0.5, 0.8, 1) * 0.01;
    vec3 inScatter = vec3(0.5, 0.8, 1) * 0.005;
    col *= exp(extinction * (-dist2Ground(altitude, viewDir)));
    col += 1 - exp(inScatter * (-dist2Ground(altitude, viewDir)));
    FragColor = vec4(col, 1.0);
}
