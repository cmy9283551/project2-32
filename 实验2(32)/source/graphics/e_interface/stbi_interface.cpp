#include "graphics/e_interface/stbi_interface.h"

void set_flip_vertically_on_load(bool flag_true_if_should_flip){
	stbi_set_flip_vertically_on_load(flag_true_if_should_flip);
}

unsigned char* image_load(char const* filename, int* x, int* y, int* comp, int req_comp){
	return stbi_load(filename,x,y,comp,req_comp);
}

void image_free(void* retval_from_stbi_load){
	stbi_image_free(retval_from_stbi_load);
}
