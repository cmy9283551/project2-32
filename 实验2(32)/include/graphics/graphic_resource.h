#pragma once

#include <map>
#include <memory>

#include "e_interface/gl_texture.h"
#include "e_interface/gl_shader.h"

class GraphicResourceManager {
public:
	GraphicResourceManager() = default;

	std::shared_ptr<Texture2D> texture2d(const std::string& file_path);
	std::shared_ptr<Shader> shader(const std::string& file_path);
private:
	class TextureManager {
	public:
		std::shared_ptr<Texture2D> texture2d(const std::string& file_path);
	private:
		std::map<std::string, std::shared_ptr<Texture2D>> texture2d_;
	};

	class ShaderManager {
	public:
		std::shared_ptr<Shader> shader(const std::string& file_path);
	private:
		std::map<std::string, std::shared_ptr<Shader>> shader_;
	};

	TextureManager texture_manager_;
	ShaderManager shader_manager_;
};

static GraphicResourceManager GraphicResourceManagerInstance;