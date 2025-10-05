#include "graphics/graphic_enum.h"

#include "tool/debugtool.h"

unsigned int GetTypeSize(GraphicDataEnum type){
	{
		switch (type)
		{
		case GraphicDataEnum::Byte:
			return sizeof(char);
		case GraphicDataEnum::UnsignedByte:
			return sizeof(unsigned char);
		case GraphicDataEnum::Short:
			return sizeof(short);
		case GraphicDataEnum::UnsignedShort:
			return sizeof(unsigned short);
		case GraphicDataEnum::Int:
			return sizeof(int);
		case GraphicDataEnum::UnsignedInt:
			return sizeof(unsigned int);
		case GraphicDataEnum::Float:
			return sizeof(float);
		case GraphicDataEnum::Double:
			return sizeof(double);
		default:
			ASSERT(false);
			break;
		}
		return 0;
	}
}
