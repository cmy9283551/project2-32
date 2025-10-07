#include "graphics/graphic_resource.h"

GraphicResourceManager::Texture2DProxy::Texture2DProxy(
	std::size_t pointer, const GraphicResourceManager& manager
) :pointer_(pointer), manager_(&manager) {
}

const Texture2D& GraphicResourceManager::Texture2DProxy::resource()const {
	return *manager_->texture_manager_->texture2d_[pointer_];
}

GraphicResourceManager::ShaderProxy::ShaderProxy(
	std::size_t pointer, const GraphicResourceManager& manager
) :pointer_(pointer), manager_(&manager) {
}

const Shader& GraphicResourceManager::ShaderProxy::resource()const {
	return *manager_->shader_manager_->shader_[pointer_];
}

void GraphicResourceManager::ShaderProxy::set_uniform1i(const std::string& name, int value) const {
	manager_->shader_manager_->shader_[pointer_]->set_uniform1i(name, value);
}

void GraphicResourceManager::ShaderProxy::set_uniform_arrayi(
	const std::string& name, const std::vector<int>& data
) const {
	manager_->shader_manager_->shader_[pointer_]->set_uniform_arrayi(name, data);
}

void GraphicResourceManager::ShaderProxy::set_uniform1f(
	const std::string& name, float value
) const {
	manager_->shader_manager_->shader_[pointer_]->set_uniform1f(name, value);
}

void GraphicResourceManager::ShaderProxy::set_uniform4f(
	const std::string& name, float v0, float v1, float v2, float v3
) const {
	manager_->shader_manager_->shader_[pointer_]->set_uniform4f(name, v0, v1, v2, v3);
}

void GraphicResourceManager::ShaderProxy::set_uniform_mat4f(
	const std::string& name, mat4 matrix
) const {
	manager_->shader_manager_->shader_[pointer_]->set_uniform_mat4f(name, matrix);
}

void GraphicResourceManager::ShaderProxy::set_uniform_array_mat4f(
	const std::string& name, const std::vector<mat4>& data
) const {
	manager_->shader_manager_->shader_[pointer_]->set_uniform_array_mat4f(name, data);
}

GraphicResourceManager::GraphicResourceManager()
	:texture_manager_(std::make_unique<TextureManager>(*this)), 
	shader_manager_(std::make_unique<ShaderManager>(*this)) {
}

GraphicResourceManager::Texture2DProxy GraphicResourceManager::texture2d(
	const std::string& file_path
) {
	return texture_manager_->texture2d(file_path);
}

GraphicResourceManager::ShaderProxy GraphicResourceManager::shader(const std::string& file_path) {
	return shader_manager_->shader(file_path);
}

void GraphicResourceManager::terminate() {
	texture_manager_->texture2d_.clear();
	shader_manager_->shader_.clear();
}

GraphicResourceManager::TextureManager::TextureManager(
	const GraphicResourceManager& parent_manager
) :parent_manager_(&parent_manager) {
}

GraphicResourceManager::Texture2DProxy GraphicResourceManager::TextureManager::texture2d(
	const std::string& file_path
) {
	auto iter = texture2d_.find(file_path);
	if (iter == texture2d_.end()) {
		std::size_t pos = texture2d_.emplace(
			std::piecewise_construct,
			std::make_tuple(file_path), 
			std::make_tuple(std::make_unique<Texture2D>(file_path))
		);
		return { pos,*parent_manager_ };
	}
	return { iter.position(),*parent_manager_ };
}

GraphicResourceManager::ShaderManager::ShaderManager(
	const GraphicResourceManager& parent_manager
) :parent_manager_(&parent_manager) {
}

GraphicResourceManager::ShaderProxy GraphicResourceManager::ShaderManager::shader(
	const std::string& file_path
) {
	auto iter = shader_.find(file_path);
	if (iter == shader_.end()) {
		std::size_t pos = shader_.emplace(
			std::piecewise_construct,
			std::make_tuple(file_path), 
			std::make_tuple(std::make_unique<Shader>(file_path))
		);
		return { pos,*parent_manager_ };
	}
	return { iter.position(),*parent_manager_ };
}
