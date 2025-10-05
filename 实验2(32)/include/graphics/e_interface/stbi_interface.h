#pragma once

#include "stb_image/stb_image.h"

void set_flip_vertically_on_load(bool flag_true_if_should_flip);

unsigned char* image_load(char const* filename, int* x, int* y, int* comp, int req_comp);

void image_free(void* retval_from_stbi_load);