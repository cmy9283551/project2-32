#pragma once

#include "e_interface/gl_vertex.h"
#include "e_interface/gl_renderer.h"

#include "graphic_geometry.h"
#include "graphic_resource.h"

class VertexData {
	friend class GEGeometry2D;
	friend class GETexGeometry2D;
	friend class GEImage;
	friend class GESprite2D;
public:
	const std::vector<unsigned char>& vertex_data()const;
	const std::vector<unsigned int>& index_data()const;

	void set_data1f(
		const VertexBufferLayout& layout, unsigned int offset,
		float value
	);
	void set_data2f(
		const VertexBufferLayout& layout, unsigned int offset,
		float value0, float value1
	);
	void set_data4f(
		const VertexBufferLayout& layout, unsigned int offset,
		float value0, float value1, float value2, float value3
	);

	void push_back(const VertexData& data);
private:
	void check(
		const VertexBufferLayout& layout,
		unsigned int offset, GraphicDataEnum type, unsigned int count
	);
	std::vector<unsigned char> vertex_data_;
	std::vector<unsigned int> index_data_;
};

struct Bound {
	float left, right, top, bottom;

	//返回a,b的交集
	static Bound min(const Bound& a, const Bound& b);
};

struct DrawCallMessage {
	unsigned int window_width, window_height;
	vec2 position;
	Bound bound;
};

enum class GEType {
	Geometry2D,
	TexGeometry2D,
	Image,
	Sprite2D,
};

class GraphicElement {
public:
	using Texture2DProxy = GraphicResourceManager::Texture2DProxy;
	using ShaderProxy = GraphicResourceManager::ShaderProxy;

	GraphicElement(GraphicResourceManager& resource_manager);
	virtual ~GraphicElement() = default;

	virtual GEType type()const = 0;
	virtual void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const = 0;

	const Shader& shader();
	void bind_shader()const;
protected:
	ShaderProxy shader_;
	GraphicResourceManager& resource_manager_;
};

class GEGeometry2D :public GraphicElement {
public:

#ifdef GRAPHIC_DEBUG
	GEGeometry2D(
		GraphicResourceManager& resource_manager,
		const std::shared_ptr<Geometry2D>& geometry2d = std::shared_ptr<Triangle>(
			new Triangle{
				{{-1.0f * MainWindowWidth / 4, 0.0f}} ,
				{{0.0f, 1.0f * MainWindowHeight / 2}} ,
				{{1.0f * MainWindowWidth / 4 , 0.0f}}
			}
		),
		const vec4& color = { 0.0f, 0.9f, 0.7f, 0.5f },
		const std::string& shader_file_path = "resource/graphics/shaders/geometry2d.shader"
	);
#else
	GEGeometry2D(
		GraphicResourceManager& resource_manager,
		const std::shared_ptr<Geometry2D>& geometry2d,
		const glm::vec4& color,
		const std::string& shader_file_path
	);
#endif // GRAPHIC_DEBUG

	~GEGeometry2D() = default;

	GEType type()const override;
	void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const override;

	void set_uniform(
		unsigned int window_width, unsigned int window_height
	)const;
	static const VertexBufferLayout& layout();
	//将数据传入数组
	unsigned int vertex_size()const;
	void data(VertexData& vertex_data,
		float pos_x, float pos_y, const Bound& bound)const;
private:
	static GEType type_;
	static VertexBufferLayout layout_;

	std::shared_ptr<Geometry2D> geometry2d_;
	vec4 color_;
};

class GETexGeometry2D :public GraphicElement {
public:

#ifdef GRAPHIC_DEBUG
	GETexGeometry2D(
		GraphicResourceManager& resource_manager,
		const std::shared_ptr<Geometry2D>& geometry2d = std::shared_ptr<Triangle>(
			new Triangle{
				{{-1.0f * MainWindowWidth / 4, 0.0f}} ,
				{{0.0f, 1.0f * MainWindowHeight / 2}} ,
				{{1.0f * MainWindowWidth / 4 , 0.0f}}
			}
		),
		const std::shared_ptr<Geometry2D>& tex_coord = std::shared_ptr<Triangle>(
			new Triangle{
				{{1.0f * MainWindowWidth / 4, 0.0f}} ,
				{{1.0f * MainWindowWidth / 2, 1.0f * MainWindowHeight / 2}} ,
				{{1.0f * MainWindowWidth / 4 * 3, 0.0f}}
			}
		),
		const vec4& color = { 0.0f, 0.9f, 0.7f, 0.5f },
		const std::vector<std::string>& texture2d_path = {
			{"resource/graphics/texture/forest.png"}
		},
		const std::string& shader_file_path = "resource/graphics/shaders/tex_geometry2d.shader"
	);
#else
	GETexGeometry2D(
		GraphicResourceManager& resource_manager,
		const std::shared_ptr<Geometry2D>& geometry2d,
		const std::shared_ptr<Geometry2D>& tex_coord,
		const glm::vec4& color,
		const std::vector<std::string>& texture2d_path,
		const std::string& shader_file_path
	);
#endif // GRAPHIC_DEBUG

	~GETexGeometry2D() = default;

	GEType type()const override;
	void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const override;

	void set_uniform(
		unsigned int window_width, unsigned int window_height,
		unsigned int available_texture_units
	)const;
	static const VertexBufferLayout& layout();

	unsigned int vertex_size()const;
	void data(VertexData& vertex_data,
		float pos_x, float pos_y, const Bound& bound)const;
	void bind_texture(unsigned int index_start, unsigned int index_end)const;
	unsigned int texture_size()const;
private:
	static GEType type_;
	static VertexBufferLayout layout_;

	std::shared_ptr<Geometry2D> geometry2d_;
	std::shared_ptr<Geometry2D> tex_coord_;
	vec4 color_;
	std::vector<Texture2DProxy> texture2d_;
};

//图像元素,形状为矩形,有且仅有一个纹理
class GEImage :public GraphicElement {
public:
#ifdef GRAPHIC_DEBUG
	GEImage(
		GraphicResourceManager& resource_manager,
		const std::shared_ptr<Rectangle>& rectangle = std::shared_ptr<Rectangle>(
			new Rectangle{ 300.0f,200.0f ,{0.0f,0.0f} }
		),
		const std::shared_ptr<Rectangle>& tex_coord = std::shared_ptr<Rectangle>(
			new Rectangle{ 1200.0f,800.0f ,{600.0f,400.0f} }
		),
		const vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		const std::string& texture2d_path = "resource/graphics/texture/forest.png",
		const std::string& shader_file_path = "resource/graphics/shaders/image.shader"	
	);
#else
	GEImage(
		GraphicResourceManager& resource_manager,
		const std::shared_ptr<Rectangle>& rectangle,
		const std::shared_ptr<Rectangle>& tex_coord,
		const glm::vec4& color,
		const std::string& texture2d_path,
		const std::string& shader_file_path
	);
#endif // GRAPHIC_DEBUG

	~GEImage() = default;

	GEType type()const override;
	void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const override;

	void set_uniform(
		unsigned int window_width, unsigned int window_height,
		unsigned int available_texture_units
	) const;
	static const VertexBufferLayout& layout();

	unsigned int vertex_size()const;
	void data(VertexData& vertex_data,
		float pos_x, float pos_y, const Bound& bound)const;
	void bind_texture(unsigned int texture_index)const;

	float width()const;
	float height()const;
private:
	static GEType type_;
	static VertexBufferLayout layout_;

	std::shared_ptr<Rectangle> rectangle_;
	std::shared_ptr<Rectangle> tex_coord_;//纹理坐标
	vec4 color_;
	Texture2DProxy texture2d_;
};

class GESprite2D :public GraphicElement {
public:
	virtual ~GESprite2D() = default;

	GEType type()const override;
private:
	static GEType type_;
};