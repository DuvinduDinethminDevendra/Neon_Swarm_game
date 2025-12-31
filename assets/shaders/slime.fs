#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float squashFactor;

void main() {
    // Calculate distance from center (0.5, 0.5)
    vec2 center = vec2(0.5, 0.5);
    vec2 dist = fragTexCoord - center;
    
    // Adjust distance for squash/stretch
    dist.y /= (1.0 - squashFactor);
    dist.x /= (1.0 + squashFactor * 0.5);
    
    float radius = length(dist);
    
    // Soft edge alpha masking (the "Blob" effect)
    float alpha = smoothstep(0.48, 0.45, radius);
    
    vec4 texel = texture(texture0, fragTexCoord);
    finalColor = texel * fragColor * alpha;
}
