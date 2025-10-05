#pragma once

enum class GraphicDataEnum {
	Null,
	Byte,
	UnsignedByte,
	Short,
	UnsignedShort,
	Int,
	UnsignedInt,
	Float,
	Double
};

unsigned int GetTypeSize(GraphicDataEnum type);