#include "graphics/e_interface/glm_interface.h"

mat4 ortho(float left, float right, float bottom, float top){
    return glm::ortho(left,right,bottom,top);
}
