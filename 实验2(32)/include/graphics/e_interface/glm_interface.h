#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;

mat4 ortho(float left, float right, float bottom, float top);