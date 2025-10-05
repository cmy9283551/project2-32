#pragma once

#include "e_interface/glfw_window.h"
#include "graphic_element.h"
#include "gui_data.h"

class GUIElement {
public:
	GUIElement();
	virtual ~GUIElement() = 0;
	virtual void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const = 0;
	virtual void handle_event(const Event& event, const GLWindow& window) = 0;

	void set_position(const vec2& position);
protected:
	bool should_display()const;

	std::string name_;
	vec2 position_;
	GUIDataManager& data_manager_;
};

class GUIContainer :public GUIElement {
	//此类中坐标为background几何中心
public:
	~GUIContainer() = default;

	void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const override;
	void handle_event(const Event& event, const GLWindow& window) override;

	void insert(const std::shared_ptr<GUIElement>& gui_element);
	Bound bound()const;
private:
	class Manager {

	};

	std::shared_ptr<GEImage> background_;
	std::vector<std::shared_ptr<GUIElement>> gui_element_;
};

class GUIIcon :public GUIElement {
	//符号,图标
public:
	void draw(
		const Renderer& renderer,
		const DrawCallMessage& message
	)const override;
	void handle_event(const Event& event, const GLWindow& window)override;
private:
	std::shared_ptr<GEGeometry2D> icon_;
};

class GUITextColumn :public GUIElement {
public:
private:
};

class GUIButton :public GUIElement {
public:
private:

};