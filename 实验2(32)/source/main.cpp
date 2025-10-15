#include <iostream>
#include <sstream>
#include <fstream>

#include "cms_window.h"

#include "complex_tool/script_tool/variable_manager.h"
#include "complex_tool/script_tool/ses_implementation/ses_parser.h"
#include "complex_tool/script_tool/SES_implementation/ses_ast_visitor.h"

static void graphics_debug() {
	GLWindow window(1200, 800, "Halle World", false);
	{
		GraphicResourceManager GRM;
		Renderer renderer;

		renderer.enable_blend();

		std::shared_ptr<GraphicElement> triangle =
			std::make_shared<GEGeometry2D>(GRM);

		std::shared_ptr<GraphicElement> tex_triangle =
			std::make_shared<GETexGeometry2D>(GRM);

		std::shared_ptr<GraphicElement> image =
			std::make_shared<GEImage>(GRM);

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
		GRM.terminate();
	}
	GLWindow::terminate();
}

static void variable_manager_debug() {
	std::string struct_data =
		"TypeA:int data0,float data1;"
		"TypeB:string str,typeA type_a;"
		;
	BasicVariableManager BVM("BVM", struct_data);
	BVM.print_struct_data(std::cout);

	auto ptr1 = BVM.create_variable("int", "value_1");
	auto ptr2 = BVM.create_variable("float", "value_2");
	auto ptr3 = BVM.create_variable("char", "value_3");
	auto ptr4 = BVM.create_variable("string", "str");
	auto ptr5 = BVM.create_variable("vector_int", "vec_i");
	auto ptr6 = BVM.create_variable("vector_float", "vec_f");
	auto ptr7 = BVM.create_variable("TypeA", "type_a");
	auto ptr8 = BVM.create_variable("TypeB", "type_b");
	auto ptr9 = BVM.create_variable("package", "package_1");

	ptr1.value().modify_int(123);
	ptr2.value().modify_float(3.14f);

	ptr3.value().modify_char('H');
	*ptr4.value().string_data().value() = "Hello World";
	ptr9.value().create_member("int", "p_int");
	ptr9.value().create_member("TypeB", "type_b");

	BVM.print_heap_data(std::cout);
}

static void ses_compile_debug() {
	using namespace ses;
	std::string struct_data =
		"TypeA:int data0,float data1;"
		"TypeB:string str,TypeA type_a;"
		"TypeC:TypeB type_b,TypeA type_a;"
		;
	BasicVariableManager BVM("BasicVariableManager", struct_data);
	BasicVariableManager BVM1("BasicVariableManager1", struct_data);
	BasicFunctionManager BFM("BasicFunctionManager");
	BVM.print_struct_data(std::cout);

	auto ptr1 = BVM.create_variable("int", "value_1");
	auto ptr2 = BVM.create_variable("float", "value_2");
	auto ptr3 = BVM.create_variable("char", "value_3");
	auto ptr4 = BVM.create_variable("string", "str");
	auto ptr5 = BVM.create_variable("vector_int", "vec_i");
	auto ptr6 = BVM.create_variable("vector_float", "vec_f");
	auto ptr7 = BVM.create_variable("TypeA", "type_a");
	auto ptr8 = BVM.create_variable("TypeB", "type_b");
	auto ptr9 = BVM.create_variable("package", "package_1");

	ptr1.value().modify_int(123);
	ptr2.value().modify_float(3.14f);
	ptr3.value().modify_char('H');
	*ptr4.value().string_data().value() = "Hello World";
	ptr9.value().create_member("int", "p_int");
	ptr9.value().create_member("TypeB", "type_b");

	BVM.print_heap_data(std::cout);

	std::string script_path = "resource/global/script/SES/test.ses";

	ScopeVisitor scope = {
		{
			&BVM,
			&BVM1
		},{
			&BFM
		}
	};


	ModuleManager module_manager;
	ModuleVisitor visitor;

	ScriptConfig default_script_config;
	ModuleConfig default_module_config;

	ParserDependence dependence = {
		&scope,
		&visitor,
		&default_script_config,
		&default_module_config
	};
	ScriptParser parser(dependence);
	auto result = parser.parse(script_path);
	std::size_t size = result->size();
	DebugASTVisitor debug_visitor(std::cout);
	for (std::size_t i = 0; i < size; i++) {
		(*result)[i]->visit(debug_visitor);
	}
}

static void temp_debug() {
	IndexedMap<std::string, std::size_t> indexed_map = { {
		{"a",1},
		{"b",2},
		{"c",3},
		{"d",4},
		{"e",5},
		{"f",3},
		{"g",5},
		{"h",4}
		}
	};
	indexed_map.erase("d");
	indexed_map.unordered_erase("a");
	indexed_map.sort([](const auto& x1, const auto& x2) {
		return x1 < x2;
		});
	auto visitors = indexed_map.get_visitor();
	std::size_t size = visitors.size();
	for (std::size_t i = 0; i < size; i++) {
		std::cout << *visitors[i].first << "=" << *visitors[i].second << "\n";
	}
}

int main() {
	//graphics_debug();
	//variable_manager_debug();
	ses_compile_debug();
	//temp_debug();
	return 0;
}