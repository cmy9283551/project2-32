#include "graphics/graphic_resource.h"

std::shared_ptr<Texture2D> GraphicResourceManager::TextureManager::texture2d(
	const std::string& file_path
) {
	auto iter = texture2d_.find(file_path);
	if (iter == texture2d_.end()) {
		std::shared_ptr<Texture2D> ptr(new Texture2D(file_path));
		texture2d_.insert({ file_path, ptr });
		return ptr;
	}
	return iter->second;
}

std::shared_ptr<Shader> GraphicResourceManager::ShaderManager::shader(
	const std::string& file_path
) {
	auto iter = shader_.find(file_path);
	if (iter == shader_.end()) {
		std::shared_ptr<Shader> ptr(new Shader(file_path));
		shader_.insert({ file_path, ptr });
		return ptr;
	}
	return iter->second;
}

std::shared_ptr<Texture2D> GraphicResourceManager::texture2d(const std::string& file_path) {
	return texture_manager_.texture2d(file_path);
}

std::shared_ptr<Shader> GraphicResourceManager::shader(const std::string& file_path) {
	return shader_manager_.shader(file_path);
}