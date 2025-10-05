#include "graphics/gui_element.h"

void GUIElement::set_position(const vec2& position) {
	position_ = position;
}

bool GUIElement::should_display() const {
	return true;
}

void GUIContainer::draw(
	const Renderer& renderer,
	const DrawCallMessage& message
) const {
	if (should_display() == false) {
		return;
	}

	std::size_t size = gui_element_.size();
	Bound min_bound = Bound::min(message.bound, this->bound());
	DrawCallMessage new_message(message);
	new_message.position = message.position + position_;


	for (std::size_t i = 0; i < size; i++) {
		gui_element_[i]->draw(renderer, new_message);
	}
}

GUIElement::GUIElement()
	:data_manager_(GUIDataManagerInstance) {
}

void GUIContainer::handle_event(const Event& event, const GLWindow& window) {
	//TO DO
}

void GUIContainer::insert(const std::shared_ptr<GUIElement>& gui_element) {
	gui_element_.push_back(gui_element);
}

Bound GUIContainer::bound()const {
	return {
		position_.x - background_->width() / 2,
		position_.x + background_->width() / 2,
		position_.y + background_->height() / 2,
		position_.y - background_->height() / 2
	};
}

void GUIIcon::draw(
	const Renderer& renderer,
	const DrawCallMessage& message
) const {
	if (should_display() == false) {
		return;
	}

	DrawCallMessage new_message(message);
	new_message.position = message.position + position_;
	icon_->draw(renderer, new_message);
}

void GUIIcon::handle_event(const Event& event, const GLWindow& window) {
	//TO DO
}
