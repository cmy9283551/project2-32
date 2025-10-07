#pragma once

#include <map>
#include <memory>

#include "e_interface/gl_texture.h"
#include "e_interface/gl_shader.h"
#include "tool/container.hpp"

//图形界面暂时不考虑多线程
class GraphicResourceManager {
public:
	GraphicResourceManager();

	class Texture2DProxy {
	public:
		Texture2DProxy() = default;
		Texture2DProxy(
			std::size_t pointer,
			const GraphicResourceManager& manager
		);

		const Texture2D& resource()const;
	private:
		std::size_t pointer_ = 0;
		const GraphicResourceManager* manager_ = nullptr;
	};
	class ShaderProxy {
	public:
		ShaderProxy() = default;
		ShaderProxy(
			std::size_t pointer, const GraphicResourceManager& manager
		);

		const Shader& resource()const;

		void set_uniform1i(const std::string& name, int value)const;
		void set_uniform_arrayi(
			const std::string& name, const std::vector<int>& data
		)const;
		void set_uniform1f(const std::string& name, float value)const;
		void set_uniform4f(
			const std::string& name, float v0, float v1, float v2, float v3
		)const;
		void set_uniform_mat4f(const std::string& name, mat4 matrix)const;
		void set_uniform_array_mat4f(
			const std::string& name, const std::vector<mat4>& data
		)const;
	private:

		std::size_t pointer_ = 0;
		const GraphicResourceManager* manager_ = nullptr;
	};

	Texture2DProxy texture2d(const std::string& file_path);
	ShaderProxy shader(const std::string& file_path);
	void terminate();
private:
	class TextureManager;
	class ShaderManager;

	std::unique_ptr<TextureManager> texture_manager_;
	std::unique_ptr<ShaderManager> shader_manager_;
};

class GraphicResourceManager::TextureManager {
	friend GraphicResourceManager;
	friend GraphicResourceManager::Texture2DProxy;
public:
	TextureManager(const GraphicResourceManager& parent_manager);

	Texture2DProxy texture2d(const std::string& file_path);
private:
	IndexedMap<std::string, std::unique_ptr<Texture2D>> texture2d_;
	const GraphicResourceManager* parent_manager_;
};

class GraphicResourceManager::ShaderManager {
	friend GraphicResourceManager;
	friend GraphicResourceManager::ShaderProxy;
public:
	ShaderManager(const GraphicResourceManager& parent_manager);

	ShaderProxy shader(const std::string& file_path);
private:
	IndexedMap<std::string, std::unique_ptr<Shader>> shader_;
	const GraphicResourceManager* parent_manager_;
};