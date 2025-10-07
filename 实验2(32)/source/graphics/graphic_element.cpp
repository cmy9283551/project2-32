#include "graphics/graphic_element.h"
#include "graphics/graphic_enum.h"

#include <algorithm>


const std::vector<unsigned char>& VertexData::vertex_data() const {
	return vertex_data_;
}

const std::vector<unsigned int>& VertexData::index_data() const {
	return index_data_;
}

void VertexData::set_data1f(
	const VertexBufferLayout& layout, unsigned int offset, float value
) {
#ifdef  GRAPHIC_DEBUG
	check(layout, offset, GraphicDataEnum::Float, 1);
#endif //  GRAPHIC_DEBUG

	for (std::size_t i = offset; i < vertex_data_.size(); i += layout.vertex_size()) {
		std::memcpy(&vertex_data_[i], &value, sizeof(float));
	}
}

void VertexData::set_data2f(
	const VertexBufferLayout& layout, unsigned int offset,
	float value0, float value1
) {
#ifdef  GRAPHIC_DEBUG
	check(layout, offset, GraphicDataEnum::Float, 2);
#endif //  GRAPHIC_DEBUG

	float data[2] = { value0,value1 };
	for (std::size_t i = offset; i < vertex_data_.size(); i += layout.vertex_size()) {
		std::memcpy(&vertex_data_[i], &data[0], 2 * sizeof(float));
	}
}

void VertexData::set_data4f(
	const VertexBufferLayout& layout, unsigned int offset,
	float value0, float value1, float value2, float value3
) {
#ifdef  GRAPHIC_DEBUG
	check(layout, offset, GraphicDataEnum::Float, 4);
#endif //  GRAPHIC_DEBUG

	float data[4] = { value0,value1,value2,value3 };
	for (std::size_t i = offset; i < vertex_data_.size(); i += layout.vertex_size()) {
		std::memcpy(&vertex_data_[i], &data[0], 4 * sizeof(float));
	}

}

void VertexData::push_back(const VertexData& data) {
	std::size_t v_size = data.vertex_data_.size(), i_size = index_data_.size();
	for (std::size_t i = 0; i < v_size; i++) {
		vertex_data_.push_back(data.vertex_data_[i]);
	}
	for (std::size_t i = 0; i < i_size; i++) {
		index_data_.push_back(data.index_data_[i]);
	}
}

void VertexData::check(
	const VertexBufferLayout& layout,
	unsigned int offset, GraphicDataEnum type, unsigned int count
) {
	if (vertex_data_.size() < layout.vertex_size()) {
		GRAPHIC_CERR << "写入错误 : 顶点数据空间不足" << std::endl;
		ASSERT(false);
		return;
	}
	if (offset + GetTypeSize(type) > layout.vertex_size()) {
		GRAPHIC_CERR << "写入错误 : 偏移量越界" << std::endl;
		ASSERT(false);
		return;
	}
	auto elements = layout.get_elements();
	bool find = false;
	for (std::size_t i = 0, before = 0; i < elements.size(); i++) {
		if (elements[i].cms_type() == type && elements[i].count == count) {
			if (before == offset) {
				find = true;
				break;
			}
		}
		before += elements[i].size();
	}
	if (find == false) {
		GRAPHIC_CERR << "写入错误 : 顶点数据格式不匹配" << std::endl;
		ASSERT(false);
		return;
	}
}

Bound Bound::min(const Bound& a, const Bound& b) {
	return {
		std::max(a.left,b.left),
		std::min(a.right,b.right),
		std::min(a.top,b.top),
		std::max(a.bottom,b.bottom)
	};
}

GraphicElement::GraphicElement(GraphicResourceManager& resource_manager)
	:resource_manager_(resource_manager) {
}

const Shader& GraphicElement::shader() {
	return shader_.resource();
}

void GraphicElement::bind_shader() const {
	shader_.resource().bind();
}

GEType GEGeometry2D::type_ = GEType::Geometry2D;

VertexBufferLayout GEGeometry2D::layout_ = []() {
	VertexBufferLayout layout;
	layout.push(GraphicDataEnum::Float, 2, false);//顶点坐标
	layout.push(GraphicDataEnum::Float, 4, false);//颜色
	layout.push(GraphicDataEnum::Float, 4, false);//边界
	return layout;
	}();

GEGeometry2D::GEGeometry2D(
	GraphicResourceManager& resource_manager,
	const std::shared_ptr<Geometry2D>& geometry2d,
	const vec4& color,
	const std::string& shader_file_path
) :geometry2d_(geometry2d), color_(color),GraphicElement(resource_manager) {
	shader_ = resource_manager_.shader(shader_file_path);
}

GEType GEGeometry2D::type() const {
	return type_;
}

void GEGeometry2D::draw(
	const Renderer& renderer, const DrawCallMessage& message
) const {
	bind_shader();
	VertexData vertex_data;
	data(vertex_data, message.position.x, message.position.y, message.bound);

	set_uniform(
		message.window_width, message.window_height
	);

	VertexBuffer vb(vertex_data.vertex_data());
	IndexBuffer ib(vertex_data.index_data());
	VertexArray va;
	va.bind_buffer(vb, GEGeometry2D::layout());
	renderer.draw(va, ib, shader_.resource());
}

void GEGeometry2D::set_uniform(unsigned int window_width, unsigned int window_height) const {
	mat4 projection = ortho(
		0.0f, static_cast<float>(window_width),
		0.0f, static_cast<float>(window_height)
	);

	shader_.set_uniform_mat4f("u_MVP", projection);
}

const VertexBufferLayout& GEGeometry2D::layout() {
	return layout_;
}

unsigned int GEGeometry2D::vertex_size() const {
	return layout_.vertex_size();
}

void GEGeometry2D::data(VertexData& vertex_data,
	float pos_x, float pos_y, const Bound& bound)const {
	//设置容器
	auto& vertex = vertex_data.vertex_data_;
	auto& index = vertex_data.index_data_;
	std::size_t size = vertex.size(), count = index.size();

	std::vector<Point2D> points;
	geometry2d_->points(points, pos_x, pos_y);
	vertex.resize(size + points.size() * vertex_size());

	//顶点数据在该函数中自动传入
	geometry2d_->indices(index, count);

	//传入顶点数据
	for (std::size_t i = 0; i < points.size(); i++) {
		std::size_t before = size + i * vertex_size();
		std::memcpy(&vertex[before], &points[i], sizeof(Point2D));
		before += sizeof(Point2D);
		std::memcpy(&vertex[before], &color_, sizeof(vec4));
		before += sizeof(vec4);
		std::memcpy(&vertex[before], &bound.left, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.right, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.top, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.bottom, sizeof(float));
	}
}

GEType GETexGeometry2D::type_ = GEType::TexGeometry2D;

VertexBufferLayout GETexGeometry2D::layout_ = []() {
	VertexBufferLayout layout;
	layout.push(GraphicDataEnum::Float, 2, false);//顶点坐标
	layout.push(GraphicDataEnum::Float, 2, false);//纹理坐标
	layout.push(GraphicDataEnum::Float, 4, false);//颜色
	layout.push(GraphicDataEnum::Float, 4, false);//边界
	layout.push(GraphicDataEnum::Float, 1, false);//纹理序号(起点)
	layout.push(GraphicDataEnum::Float, 1, false);//纹理序号(终点)
	//纹理序号前闭后开
	return layout;
	}();

GETexGeometry2D::GETexGeometry2D(
	GraphicResourceManager& resource_manager,
	const std::shared_ptr<Geometry2D>& geometry2d,
	const std::shared_ptr<Geometry2D>& tex_coord,
	const vec4& color,
	const std::vector<std::string>& texture2d_path,
	const std::string& shader_file_path
) :geometry2d_(geometry2d), tex_coord_(tex_coord), color_(color),
GraphicElement(resource_manager) {
	if (geometry2d->type() != tex_coord_->type()) {
		GRAPHIC_CERR << "纹理坐标和图形顶点类型不匹配" << std::endl;
		ASSERT(false);
	}
	shader_ = resource_manager_.shader(shader_file_path);
	std::size_t size = texture2d_path.size();
	for (std::size_t i = 0; i < size; i++) {
		texture2d_.emplace_back(resource_manager_.texture2d(texture2d_path[i]));
	}
}

GEType GETexGeometry2D::type() const {
	return type_;
}

void GETexGeometry2D::draw(
	const Renderer& renderer,
	const DrawCallMessage& message
) const {
	bind_shader();
	VertexData vertex_data;
	data(vertex_data, message.position.x, message.position.y, message.bound);

	unsigned int texture_size = this->texture_size();
	if (renderer.available_texture_units() < texture_size) {
		GRAPHIC_CERR << "绘制错误 : 可用纹理单元数不足,需求" << texture_size << std::endl;
		ASSERT(false);
		return;
	}
	unsigned int tex_index_start = 0, tex_index_end = texture_size;
	//纹理序号前闭后开
	vertex_data.set_data1f(
		GETexGeometry2D::layout(),
		2 * sizeof(Point2D) + sizeof(vec4) + sizeof(Bound),
		static_cast<float>(tex_index_start)
	);
	vertex_data.set_data1f(
		GETexGeometry2D::layout(),
		2 * sizeof(Point2D) + sizeof(vec4) + sizeof(Bound) + sizeof(float),
		static_cast<float>(tex_index_end)
	);
	bind_texture(tex_index_start, tex_index_end);

	set_uniform(
		message.window_width, message.window_height,
		renderer.available_texture_units()
	);

	VertexBuffer vb(vertex_data.vertex_data());
	IndexBuffer ib(vertex_data.index_data());
	VertexArray va;
	va.bind_buffer(vb, GETexGeometry2D::layout());
	renderer.draw(va, ib, shader_.resource());
}

void GETexGeometry2D::set_uniform(
	unsigned int window_width, unsigned int window_height, unsigned int available_texture_units
) const {
	mat4 projection = ortho(
		0.0f, static_cast<float>(window_width),
		0.0f, static_cast<float>(window_height)
	);

	std::vector<int> units;
	for (std::size_t i = 0; i < available_texture_units; i++) {
		units.push_back(i);
	}

	std::vector<mat4> tex_mat;
	for (std::size_t i = 0; i < texture2d_.size(); i++) {
		tex_mat.push_back(
			ortho(
				0.0f, static_cast<float>(texture2d_[i].resource().width()),
				0.0f, static_cast<float>(texture2d_[i].resource().height())
			)
		);
	}

	shader_.set_uniform_mat4f("u_MVP", projection);
	shader_.set_uniform_arrayi("u_texture", units);
	shader_.set_uniform_array_mat4f("u_tex_mat", tex_mat);
}

const VertexBufferLayout& GETexGeometry2D::layout() {
	return layout_;
}

unsigned int GETexGeometry2D::vertex_size() const {
	return layout_.vertex_size();
}

void GETexGeometry2D::data(VertexData& vertex_data,
	float pos_x, float pos_y, const Bound& bound)const {
	auto& vertex = vertex_data.vertex_data_;
	auto& index = vertex_data.index_data_;
	std::size_t size = vertex.size(), count = index.size();

	std::vector<Point2D> points, tex_coords;
	geometry2d_->points(points, pos_x, pos_y);
	tex_coord_->points(tex_coords, 0, 0);
	vertex.resize(size + points.size() * vertex_size());

	//顶点数据在该函数中自动传入
	geometry2d_->indices(index, count);

	//传入顶点数据
	for (std::size_t i = 0; i < points.size(); i++) {
		std::size_t before = size + i * vertex_size();
		std::memcpy(&vertex[before], &points[i], sizeof(Point2D));
		before += sizeof(Point2D);
		std::memcpy(&vertex[before], &tex_coords[i], sizeof(Point2D));
		before += sizeof(Point2D);
		std::memcpy(&vertex[before], &color_, sizeof(vec4));
		before += sizeof(vec4);
		std::memcpy(&vertex[before], &bound.left, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.right, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.top, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.bottom, sizeof(float));
		before += sizeof(float);
		float tex_index_start = 0.0f, tex_index_end = 0.0f;
		std::memcpy(&vertex[before], &tex_index_start, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &tex_index_end, sizeof(float));
	}
}

void GETexGeometry2D::bind_texture(unsigned int index_start, unsigned int index_end) const {
	if (index_start >= index_end || index_end > texture2d_.size()) {
		GRAPHIC_CERR <<
			"绑定错误 : 纹理序号范围错误,范围[" << index_start << "," << index_end <<
			"),纹理数量" << texture2d_.size() << std::endl;
		ASSERT(false);
		return;
	}
	for (unsigned int i = index_start; i < index_end; i++) {
		texture2d_[i].resource().bind(i);
	}
}

unsigned int GETexGeometry2D::texture_size() const {
	return texture2d_.size();
}

GEType GEImage::type_ = GEType::Image;

VertexBufferLayout GEImage::layout_ = []() {
	VertexBufferLayout layout;
	layout.push(GraphicDataEnum::Float, 2, false);//顶点坐标
	layout.push(GraphicDataEnum::Float, 2, false);//纹理坐标
	layout.push(GraphicDataEnum::Float, 4, false);//颜色
	layout.push(GraphicDataEnum::Float, 4, false);//边界
	layout.push(GraphicDataEnum::Float, 1, false);//纹理序号(预留)
	return layout;
	}();

GEImage::GEImage(
	GraphicResourceManager& resource_manager,
	const std::shared_ptr<Rectangle>& rectangle,
	const std::shared_ptr<Rectangle>& tex_coord,
	const glm::vec4& color,
	const std::string& texture2d_path,
	const std::string& shader_file_path
) :rectangle_(rectangle), tex_coord_(tex_coord), color_(color),
GraphicElement(resource_manager) {
	shader_ = resource_manager_.shader(shader_file_path);
	texture2d_ = resource_manager_.texture2d(texture2d_path);
}

GEType GEImage::type() const {
	return type_;
}

void GEImage::draw(
	const Renderer& renderer,
	const DrawCallMessage& message
) const {
	bind_shader();
	VertexData vertex_data;
	data(vertex_data, message.position.x, message.position.y, message.bound);

	//处理纹理
	if (renderer.available_texture_units() < 1) {
		GRAPHIC_CERR << "绘制错误 : 可用纹理单元数不足,需求" << 1 << std::endl;
		ASSERT(false);
		return;
	}
	unsigned int texture_index = 0;
	vertex_data.set_data1f(
		GEImage::layout(),
		2 * sizeof(Point2D) + sizeof(vec4) + sizeof(Bound),
		static_cast<float>(texture_index)
	);
	bind_texture(texture_index);

	set_uniform(
		message.window_width, message.window_height,
		renderer.available_texture_units()
	);

	VertexBuffer vb(vertex_data.vertex_data());
	IndexBuffer ib(vertex_data.index_data());
	VertexArray va;
	va.bind_buffer(vb, GEImage::layout());
	renderer.draw(va, ib, shader_.resource());
}

void GEImage::set_uniform(
	unsigned int window_width, unsigned int window_height, unsigned int available_texture_units
) const {
	mat4 projection = ortho(
		0.0f, static_cast<float>(window_width),
		0.0f, static_cast<float>(window_height)
	);

	std::vector<int> units;
	for (std::size_t i = 0; i < available_texture_units; i++) {
		units.push_back(i);
	}

	mat4 tex_mat = ortho(
		0.0f, static_cast<float>(texture2d_.resource().width()),
		0.0f, static_cast<float>(texture2d_.resource().height())
	);

	shader_.set_uniform_mat4f("u_MVP", projection);
	shader_.set_uniform_arrayi("u_texture", units);
	shader_.set_uniform_mat4f("u_tex_mat", tex_mat);
}

const VertexBufferLayout& GEImage::layout() {
	return layout_;
}

GLuint GEImage::vertex_size() const {
	return layout_.vertex_size();
}

void GEImage::data(VertexData& vertex_data,
	float pos_x, float pos_y, const Bound& bound)const {
	auto& vertex = vertex_data.vertex_data_;
	auto& index = vertex_data.index_data_;
	std::size_t size = vertex.size(), count = index.size();

	std::vector<Point2D> points, tex_coords;
	rectangle_->points(points, pos_x, pos_y);
	tex_coord_->points(tex_coords, 0, 0);
	vertex.resize(size + points.size() * vertex_size());

	//顶点数据在该函数中自动传入
	rectangle_->indices(index, count);

	//传入顶点数据
	for (std::size_t i = 0; i < points.size(); i++) {
		std::size_t before = size + i * vertex_size();
		std::memcpy(&vertex[before], &points[i], sizeof(Point2D));
		before += sizeof(Point2D);
		std::memcpy(&vertex[before], &tex_coords[i], sizeof(Point2D));
		before += sizeof(Point2D);
		std::memcpy(&vertex[before], &color_, sizeof(vec4));
		before += sizeof(vec4);
		std::memcpy(&vertex[before], &bound.left, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.right, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.top, sizeof(float));
		before += sizeof(float);
		std::memcpy(&vertex[before], &bound.bottom, sizeof(float));
		before += sizeof(float);
		float tex_index = 0.0f;
		std::memcpy(&vertex[before], &tex_index, sizeof(float));
	}
}

void GEImage::bind_texture(unsigned int texture_index) const {
	texture2d_.resource().bind(texture_index);
}

float GEImage::width() const {
	return rectangle_->width();
}

float GEImage::height() const {
	return rectangle_->height();
}

GEType GESprite2D::type_ = GEType::Sprite2D;

GEType GESprite2D::type() const {
	return type_;
}