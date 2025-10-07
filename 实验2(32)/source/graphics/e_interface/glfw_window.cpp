#include "graphics/e_interface/glfw_window.h"

#include <iostream>
#include <vector>

EventEnum KeyEvent::event_type = EventEnum::KeyEvent;

KeyEvent::KeyEvent(KeyEnum key_, ActionEnum action_)
	:key(key_), action(action_) {
}

EventEnum KeyEvent::type() const {
	return event_type;
}

MessageManager::MessageManager()
	:key_state_(max_key_) {
}

bool MessageManager::is_key_event(
	const Event& event,
	KeyEnum key, ActionEnum action
) {
	if (event.type() != EventEnum::KeyEvent) {
		return false;
	}
	const KeyEvent& k_event = static_cast<const KeyEvent&>(event);
	if (k_event.key != key || k_event.action != action) {
		return false;
	}
	return true;
}

void MessageManager::handle_event(const Event& event) {
	if (event.type() != EventEnum::KeyEvent) {
		return;
	}
	const KeyEvent& k_event = static_cast<const KeyEvent&>(event);
	unsigned int key = unsigned int(k_event.key);
	if (key > max_key_) {
		GRAPHIC_CERR << "不存在编号为" << key << "的按键" << std::endl;
		ASSERT(false);
		return;
	}

	if (k_event.action == ActionEnum::Press) {
		key_state_[key] = true;
	}

	if (k_event.action == ActionEnum::Release) {
		key_state_[key] = false;
	}
}

bool MessageManager::check_key_state(KeyEnum key) const {
	unsigned int i_key = unsigned int(key);
	if (i_key > max_key_) {
		GRAPHIC_CERR << "不存在编号为" << i_key << "的按键" << std::endl;
		ASSERT(false);
		return false;
	}
	return key_state_[i_key];
}

GLWindow::GLWindow(
	int width, int height, const std::string& title, bool is_changeable,
	const std::function<void(const Event&)>& handle_func
) :handle_func_(handle_func) {

	init_glfw();
	init_window(width, height, title, is_changeable, NULL);
}

GLWindow::GLWindow(
	int width, int height, const std::string& title, bool is_changeable,
	const GLWindow& share,
	const std::function<void(const Event&)>& handle_func
) :handle_func_(handle_func) {

	init_glfw();
	init_window(width, height, title, is_changeable, share.window_.window);
}

GLWindow::~GLWindow() {
	if (is_closed_ == false) {
		glfwDestroyWindow(window_.window);
	}
}

void GLWindow::pull_events() {
	/* Poll for and process events */
	glfwPollEvents();
}

void GLWindow::terminate(){
	glfwTerminate();
}

bool GLWindow::should_close()const {
	return glfwWindowShouldClose(window_.window);
}

bool GLWindow::is_closed() const {
	return is_closed_;
}

void GLWindow::swap_buffers()const {
	/* Swap front and back buffers */
	//交换帧缓存
	//即用已完成的帧替换当前帧
	glfwSwapBuffers(window_.window);
}

unsigned int GLWindow::width()const {
	return window_.width;
}

unsigned int GLWindow::height()const {
	return window_.height;
}

void GLWindow::make_context() const {
	glfwMakeContextCurrent(window_.window);
}

void GLWindow::close() {
	glfwDestroyWindow(window_.window);
	is_closed_ = true;
}

void GLWindow::set_handle_func(
	const std::function<void(const Event&)>& handle_func
) {
	handle_func_ = handle_func;
}

void GLWindow::handle_event(const Event& event) {
	message_manager_.handle_event(event);
	handle_func_(event);
}

bool GLWindow::check_key_state(KeyEnum key) const {
	return message_manager_.check_key_state(key);
}

void GLWindow::init_glfw() const {
	/* Initialize the library */
	if (!glfwInit()) {
		GRAPHIC_CERR << "GLFW Error" << std::endl;
		ASSERT(false);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void GLWindow::init_window(
	int width, int height, const std::string& title, bool is_changeable,
	GLFWwindow* share
) {
	window_.width = width, window_.height = height, window_.title = title;
	/* Create a windowed mode window and its OpenGL context */
	window_.window = glfwCreateWindow(width, height, title.c_str(), NULL, share);
	if (!window_.window) {
		glfwTerminate();
		GRAPHIC_CERR << "GLFW Error" << std::endl;
		ASSERT(false);
	}

	if (!is_changeable) {
		glfwSetWindowSizeLimits(window_.window, width, height, width, height);
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window_.window);

	glfwSwapInterval(1);
	//[此函数设置当前上下文的交换间隔
	//即在交换窗口缓冲区并从glfwSwapBuffers返回之前需要等待的屏幕更新次数]官网介绍
	//不调用：驱动程序默认
	//0：不等待垂直同步（可能被驱动程序 / 驱动程序设置覆盖）
	//1：等待第一个垂直同步（可能被驱动程序 / 驱动程序设置覆盖）

	if (glewInit() != GLEW_OK) {
		GRAPHIC_CERR << "GLEW Error" << std::endl;
		ASSERT(false);
	}
	//要在glfw创建了opengl上下文之后才能调用glewInit()

	glfwSetWindowUserPointer(window_.window, &window_);
	window_.identity = GLWindowManager::push_back(this);
	//绑定回调函数
	glfwSetKeyCallback(window_.window, GLWindowManager::KeyCallBack);
}

std::vector<GLWindow*> GLWindowManager::window_container_;

void GLWindowManager::KeyCallBack(
	GLFWwindow* window, int key, int scancode, int action, int mods
) {
	GLWindow::GLWindowData* data =
		static_cast<GLWindow::GLWindowData*>(glfwGetWindowUserPointer(window));
	if (!data) {
		GRAPHIC_CERR << "KeyCallBack:窗口数据不存在" << std::endl;
		ASSERT(false);
	}
	std::shared_ptr<Event> event(
		new KeyEvent(
			GLWindowManager::key_transform(key),
			GLWindowManager::action_transform(action)
		)
	);
	GLWindowManager::handle_event(data->identity, *event);
}

KeyEnum GLWindowManager::key_transform(int key) {
	return KeyEnum(key);
}

ActionEnum GLWindowManager::action_transform(int action) {
	switch (action)
	{
	case GLFW_PRESS:
		return ActionEnum::Press;
	case GLFW_RELEASE:
		return ActionEnum::Release;
	case GLFW_REPEAT:
		return ActionEnum::Repeat;
	default:
		break;
	}
	GRAPHIC_CERR << "不存在操作" << action << std::endl;
	ASSERT(false);
	return ActionEnum::Null;
}

std::size_t GLWindowManager::push_back(GLWindow* window) {

	window_container_.push_back(window);
	return window_container_.size() - 1;
}

void GLWindowManager::handle_event(
	std::size_t index, const Event& event
) {
	window_container_[index]->handle_event(event);
}