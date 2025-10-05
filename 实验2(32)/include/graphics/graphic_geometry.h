#pragma once

#include <vector>

#include "e_interface/graphic_tool.h"

//geometry的任务是将图形拆分为顶点和索引
//一切图形的中心坐标为(0.0 ,0.0 ,0.0),中心坐标为旋转中心
//实际坐标为位置坐标+中心坐标+图形顶点坐标

enum class GeometryType {
	Triangle,
	Rectangle,
};

struct Point2D {
	vec2 position;
};

class Geometry2D {
public:
	virtual ~Geometry2D() = default;

	virtual GeometryType type()const = 0;
	virtual void points(std::vector<Point2D>& point, float x, float y)const = 0;
	virtual void indices(std::vector<unsigned int>& indeices, unsigned int move = 0)const = 0;
private:
};

class Triangle :public Geometry2D {
public:
	Triangle(const Point2D& a, const Point2D& b, const Point2D& c);
	~Triangle() = default;

	GeometryType type()const override;
	void points(std::vector<Point2D>& point, float x, float y)const override;
	void indices(std::vector<unsigned int>& indeices, unsigned int move = 0)const override;
private:
	static GeometryType type_;

	Point2D data_[3];
};

class Rectangle :public Geometry2D {
public:
	Rectangle(float width, float height, const vec2& center);
	~Rectangle() = default;

	GeometryType type()const override;
	void points(std::vector<Point2D>& point, float x, float y)const override;
	void indices(std::vector<unsigned int>& indeices, unsigned int move = 0)const override;

	float width()const;
	float height()const;
private:
	static GeometryType type_;

	float width_, height_;
	vec2 center_;
};

struct Point3D {
	vec3 position;
};

class Geometry3D {
public:
	virtual ~Geometry3D() = 0;
};