#ifndef SkyBox_hpp
#define SkyBox_hpp

#include <stdio.h>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "Shader.hpp"

namespace gps {
    class SkyBox {
    public:
        SkyBox();
        void Load(std::vector<const GLchar*> cubeFaces);
        void Draw(gps::Shader shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    private:
        GLuint skyboxVAO;
        GLuint skyboxVBO;
        GLuint textureID;
        void Init();
        GLuint LoadSkyBoxTextures(std::vector<const GLchar*> cubeFaces);
    };
}

#endif /* SkyBox_hpp */