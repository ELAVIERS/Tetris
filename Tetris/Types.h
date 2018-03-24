#pragma once

/*
	Types.h

	Just typedefs I want in one place, nothing to see here...
*/

#define SQUARE(X) X * X

#define BYTE_MAX 255

typedef unsigned __int8		byte;
typedef signed __int16		int16;
typedef signed __int32		int32;
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;

inline int16 BufferToInt16(const byte buffer[2]) {
	return buffer[0] + (buffer[1] << 8);
}

inline void Int16ToBuffer(int16 value, byte buffer[2]) {
	buffer[0] = (value & 0x00FF);
	buffer[1] = (value & 0xFF00) >> 8;
}

inline int32 BufferToInt32(const byte buffer[4]) {
	return buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24);
}

inline void Int32ToBuffer(int32 value, byte buffer[4]) {
	buffer[0] = (value & 0x000000FF);
	buffer[1] = (value & 0x0000FF00) >> 8;
	buffer[2] = (value & 0x00FF0000) >> 16;
	buffer[3] = (value & 0xFF000000) >> 24;
}
