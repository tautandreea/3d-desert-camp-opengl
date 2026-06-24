#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

// matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

// lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 pointLightPos;     
uniform vec3 pointLightColor;
uniform float pointLightKc;
uniform float pointLightKl;
uniform float pointLightKq;
uniform float blinnShininess;
uniform float blinnSpecStrength;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform vec3 baseColor;
uniform bool useTexture;
uniform int activateFog;
uniform int useFlat; // 0 = smooth, 1 = flat


// components (dir)
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

// components (point)  
vec3 ambientPoint;
vec3 diffusePoint;
vec3 specularPoint;

float computeFog(vec4 fPosEyeLocal)
{
    float fogDensity = 0.2f; 
    float fragmentDistance = length(fPosEyeLocal.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

void computeDirLight(vec4 fPosEye, vec3 normalEye, vec3 viewDir)
{
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    ambient = ambientStrength * lightColor;
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0);
    specular = specularStrength * specCoeff * lightColor;
}

void computePointLight(vec4 fPosEye, vec3 normalEye, vec3 viewDir)
{
    vec3 lightVec = pointLightPos - fPosEye.xyz;
    float distance = length(lightVec);
    vec3 lightDirP = normalize(lightVec);

    float attenuation =
        1.0 /
        (pointLightKc +
         pointLightKl * distance +
         pointLightKq * distance * distance);

    ambientPoint = 0.01f * pointLightColor * attenuation;

    float diff = max(dot(normalEye, lightDirP), 0.0f);
    diffusePoint = diff * pointLightColor * attenuation;

    vec3 halfwayDir = normalize(lightDirP + viewDir);
    float spec = pow(max(dot(normalEye, halfwayDir), 0.0f), blinnShininess);
    specularPoint = blinnSpecStrength * spec * pointLightColor * attenuation;
}

void main()
{
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal); // SMOOTH (interpolat)

    if (useFlat == 1) {
       normalEye = normalize(cross(dFdx(fPosEye.xyz), dFdy(fPosEye.xyz)));
    }

    vec3 viewDir = normalize(-fPosEye.xyz);

    computeDirLight(fPosEye, normalEye, viewDir);
    computePointLight(fPosEye, normalEye, viewDir);

    vec3 diffColor = useTexture ? texture(diffuseTexture, fTexCoords).rgb : baseColor;
    vec3 specColor = useTexture ? texture(specularTexture, fTexCoords).rgb : vec3(0.1);
    
    vec3 color = (ambient + diffuse) * diffColor + 
             specular * specColor +
             (ambientPoint + diffusePoint) * diffColor + 
             specularPoint * specColor;

   if (activateFog == 1) {
        float fogFactor = computeFog(fPosEye);
        vec3 fogColor = vec3(0.5f, 0.5f, 0.5f); 
        color = mix(fogColor, color, fogFactor);
    }

    fColor = vec4(color, 1.0);
}