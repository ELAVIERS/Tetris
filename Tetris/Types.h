#pragma once

/*
	Types.h

	Just typedefs I want in one place, nothing to see here...
*/

#define BYTE_MAX 255

typedef unsigned __int8		byte;
typedef signed __int16		int16;
typedef unsigned __int16	uint16;

inline int16 BufferToInt16(const byte buffer[2]) {
	return buffer[0] + (buffer[1] << 8);
}

inline void Int16ToBuffer(int16 value, byte buffer[2]) {
	buffer[0] = (value & 0x00FF);
	buffer[1] = (value & 0xFF00) >> 8;
}
