#pragma once

#include "graphics/gui_element.h"

class CMSWindow;

class WindowController {
public:
	WindowController(const CMSWindow& window);

	void handle_event(const Event& event);
private:
	const CMSWindow& window_;
};

class CMSWindow {
public:
	CMSWindow(int width, int height, const std::string& title, bool is_changeable);
	void handle_event(const Event& event);

	static std::shared_ptr<CMSWindow> main_window();
private:
	static std::shared_ptr<CMSWindow> main_window_;

	WindowController window_controller_;
	GLWindow window_;
};