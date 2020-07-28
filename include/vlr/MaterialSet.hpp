#pragma once
#include "./Config.hpp"
#include "./Driver.hpp"

namespace vlr {

#pragma pack(push, 1)
    struct MaterialUnit {
        glm::vec4 diffuse = glm::vec4(1.f);
        glm::vec4 pbrAGM = glm::vec4(0.f);
        glm::vec4 emission = glm::vec4(0.f);
        glm::vec4 normal = glm::vec4(0.5f, 0.5f, 1.f, 1.f);
        
        int diffuseTexture = -1;
        int pbrAGMTexture = -1;
        int emissionTexture = -1;
        int normalTexture = -1;
    };
#pragma pack(pop)

    using MaterialSet = SetBase_T<MaterialUnit>;

};
