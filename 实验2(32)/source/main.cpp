#include <iostream>
#include <sstream>
#include <fstream>

#include "cms_window.h"
#include "tool/debugtool.h"

#include "complex_tool/script_tool/variable_manager.h"
#include "complex_tool/script_tool/ses_tool.h"

static void graphics_debug() {
	GLWindow window(1200, 800, "Halle World", false);
	Renderer renderer;

	renderer.enable_blend();

	std::shared_ptr<GraphicElement> triangle =
		std::shared_ptr<GEGeometry2D>(new GEGeometry2D);

	std::shared_ptr<GraphicElement> tex_triangle =
		std::shared_ptr<GETexGeometry2D>(new GETexGeometry2D);

	std::shared_ptr<GraphicElement> image =
		std::shared_ptr<GEImage>(new GEImage);

	Bound text_bound{ 0.0f,850.0f,
	   500.0f,0.0f };
	Bound total_window{
		0.0f,static_cast<float>(window.width()),
	   static_cast<float>(window.height()),0.0f
	};

	DrawCallMessage message(
		window.width(), window.height(),
		{ 600.0f, 400.0f },
		text_bound
	);
	DrawCallMessage message1(
		window.width(), window.height(),
		{ 600.0f, 200.0f },
		total_window
	);
	DrawCallMessage message2(
		window.width(), window.height(),
		{ 150.0f, 100.0f },
		total_window
	);
	while (!window.should_close() && !window.is_closed()) {
		GLWindow::pull_events();
		renderer.clear(0.2f, 0.3f, 0.3f, 1.0f);

		triangle->draw(renderer, message);
		tex_triangle->draw(renderer, message1);
		image->draw(renderer, message2);

		window.swap_buffers();
	}
}

static void variable_manager_debug() {
	std::string struct_data =
		"TypeA:Int data0,Float data1;"
		"TypeB:String str,TypeA type_a;"
		;
	BasicVariableManager BVM(struct_data);
	BVM.print_struct_data();


	auto ptr1 = BVM.create_variable("Int", "value_1");
	auto ptr2 = BVM.create_variable("Float", "value_2");
	auto ptr3 = BVM.create_variable("Char", "value_3");
	auto ptr4 = BVM.create_variable("String", "str");
	auto ptr5 = BVM.create_variable("VectorInt", "vec_i");
	auto ptr6 = BVM.create_variable("VectorFloat", "vec_f");
	auto ptr7 = BVM.create_variable("TypeA", "type_a");
	auto ptr8 = BVM.create_variable("TypeB", "type_b");
	auto ptr9 = BVM.create_variable("Package", "package");

	ptr1.value().modify_int(123);
	ptr2.value().modify_float(3.14f);
	ptr3.value().modify_char('H');
	*ptr4.value().string_data().value() = "Hello World";
	ptr9.value().create_member("Int", "p_int");
	ptr9.value().create_member("TypeB", "type_b");

	BVM.print_heap_data();
}

static void ses_compile_debug() {
	std::string path = "resource/global/script/SES/test.ses";
	SESLexer lexer;
	SESTokenStream token_stream(path);
	if (lexer.tokenize(token_stream) == false) {
		GRAPHIC_CERR << "Tokenize failed" << std::endl;
		return;
	}
	std::cout << token_stream;
}

static void temp_debug() {
	IndexedMap<std::string, std::size_t> list = {
		{
			{"a",1},
			{"b",2},
			{"c",3},
			{"d",4},
			{"e",5}
		}
	};
	list.erase("c");
	list.unordered_erase(1);
	std::size_t size = list.size();
	for (std::size_t i = 0; i < size; i++) {
		auto iter = list.find(i);
		if (iter == list.end()) {
			continue;
		}
		std::cout << iter.first() << "=" << iter.second() << "\n";
	}
}

int main() {
	//graphics_debug();
	//variable_manager_debug();
	//ses_compile_debug();
	temp_debug();
	return 0;
}