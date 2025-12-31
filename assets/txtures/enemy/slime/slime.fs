#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float squashFactor; 
uniform float time; // Add a time uniform for the liquid effect

void main() {
    vec2 center = vec2(0.5, 0.5);
    
    // Add a liquid "wobble" to the texture coordinates
    vec2 uv = fragTexCoord;
    uv.x += sin(uv.y * 10.0 + time * 2.0) * 0.02;
    uv.y += cos(uv.x * 10.0 + time * 2.0) * 0.02;

    vec2 dist = fragTexCoord - center;
    
    // Physical deformation
    dist.y /= (1.0 - squashFactor);
    dist.x /= (1.0 + squashFactor * 0.5);
    
    float radius = length(dist);
    
    // Create layered "blob" edges
    float alpha = smoothstep(0.48, 0.42, radius);
    float innerGlow = smoothstep(0.40, 0.0, radius) * 0.3; // Adds depth
    
    vec4 texel = texture(texture0, uv);
    finalColor = (texel + innerGlow) * fragColor * alpha;
}