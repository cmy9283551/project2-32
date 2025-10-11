#include "tool/script_tool/script_debug_tool.h"

void print_header(std::ostream& os, const std::string& header){
	os << "====================" << header << "====================\n";
}

void print_footer(std::ostream& os, const std::string& footer){
	os << "====================" << footer << "====================\n";
}
