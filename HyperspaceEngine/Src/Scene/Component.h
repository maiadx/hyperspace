#pragma once
#include "Core/Core.h"
#include <glm/glm.hpp>



struct hyComponent
{
    void* EntityHandle;

};

struct hyTransformComponent
{
    glm::mat4 Transform;

    
};

