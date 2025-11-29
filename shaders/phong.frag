#version 410

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

struct LightInfo {
  vec4 Position; // Light position in eye coords.
  vec3 La;       // Ambient light intensity
  vec3 Ld;       // Diffuse light intensity
  vec3 Ls;       // Specular light intensity
};
uniform LightInfo Light;

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};
uniform MaterialInfo Material;

uniform bool blinn;

void main() {
    vec3 n = normalize(Normal);
    vec3 s = normalize(vec3(Light.Position) - FragPos);
    vec3 v = normalize(-FragPos);
    
    vec3 ambient = Light.La * Material.Ka;
    
    float sDotN = max(dot(s, n), 0.0);
    vec3 diffuse = Light.Ld * Material.Kd * sDotN;
    
    vec3 spec = vec3(0.0);
    if(sDotN > 0.0) {
        float specFactor = 0.0;
        if(blinn) {
            vec3 halfwayDir = normalize(s + v);
            specFactor = pow(max(dot(n, halfwayDir), 0.0), Material.Shininess);
        } else {
            vec3 r = reflect(-s, n);
            specFactor = pow(max(dot(r, v), 0.0), Material.Shininess);
        }
        spec = Light.Ls * Material.Ks * specFactor;
    }
    
    vec3 result = ambient + diffuse + spec;
    FragColor = vec4(result, 1.0);
}