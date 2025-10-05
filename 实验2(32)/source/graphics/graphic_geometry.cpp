#include "graphics/graphic_geometry.h"

GeometryType Triangle::type_ = GeometryType::Triangle;

Triangle::Triangle(const Point2D& a, const Point2D& b, const Point2D& c)
	:data_{ a,b,c } {
}

GeometryType Triangle::type() const {
	return type_;
}

void Triangle::points(std::vector<Point2D>& point, float x, float y) const {
	point.push_back({ data_[0].position + vec2(x, y) });
	point.push_back({ data_[1].position + vec2(x, y) });
	point.push_back({ data_[2].position + vec2(x, y) });
}

void Triangle::indices(std::vector<unsigned int>& indeices, unsigned int move) const {
	indeices.push_back(0 + move);
	indeices.push_back(1 + move);
	indeices.push_back(2 + move);
}

GeometryType Rectangle::type_ = GeometryType::Rectangle;

Rectangle::Rectangle(float width, float height, const vec2& center)
	:width_(width), height_(height), center_(center) {
}

GeometryType Rectangle::type() const {
	return type_;
}

void Rectangle::points(std::vector<Point2D>& point, float x, float y) const {
	float cx = center_.x + x, cy = center_.y + y;
	point.push_back({ vec2(cx - width_ / 2, cy - height_ / 2) });
	point.push_back({ vec2(cx + width_ / 2, cy - height_ / 2) });
	point.push_back({ vec2(cx + width_ / 2, cy + height_ / 2) });
	point.push_back({ vec2(cx - width_ / 2, cy + height_ / 2) });
}

void Rectangle::indices(std::vector<unsigned int>& indeices, unsigned int move) const {
	indeices.push_back(0 + move);
	indeices.push_back(1 + move);
	indeices.push_back(2 + move);
	indeices.push_back(2 + move);
	indeices.push_back(3 + move);
	indeices.push_back(0 + move);
}

float Rectangle::width() const {
	return width_;
}

float Rectangle::height() const {
	return height_;
}