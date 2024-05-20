#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPosition;
uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform vec3 planetColor;
uniform bool isSun;
uniform vec3 sunColor;
uniform float glowRadius;

void main() {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    if (isSun) {
        // Specjalne traktowanie dla Słońca
        vec3 toCenter = normalize(FragPos - lightPosition);
        float distance = length(FragPos - lightPosition);
        float glowFactor = 1.0 / (distance * distance / glowRadius);

        ambient = glowFactor * sunColor;
        diffuse = glowFactor * sunColor;
        specular = glowFactor * sunColor;
    } else {
        // Standardowe traktowanie dla planet
        // Ambient
        ambient = light.ambient * planetColor;

        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPosition - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = light.diffuse * (diff * planetColor);

        // Specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * (spec * material.specular);
    }

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
