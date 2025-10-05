#include "cms_window.h"

WindowController::WindowController(const CMSWindow& window)
	:window_(window) {

}

void WindowController::handle_event(const Event& event) {
	//TO DO
}
//TO DO
std::shared_ptr<CMSWindow> CMSWindow::main_window_ = 0;

CMSWindow::CMSWindow(int width, int height, const std::string& title, bool is_changeable)
	:window_controller_(*this), window_(width, height, title, is_changeable) {
	std::function<void(const Event& event)> handle_func =
		std::bind(&CMSWindow::handle_event, this, std::placeholders::_1);
	window_.set_handle_func(handle_func);

}

void CMSWindow::handle_event(const Event& event) {

	window_controller_.handle_event(event);
	//TO DO
}

std::shared_ptr<CMSWindow> CMSWindow::main_window() {
	return main_window_;
}