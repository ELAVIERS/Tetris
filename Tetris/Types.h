#pragma once

/*
	Types.h

	Just typedefs I want in one place, nothing to see here...
*/

typedef unsigned __int8		byte;
typedef unsigned __int16	uint16;

inline uint16 BufferToUint16(const byte buffer[2]) {
	return buffer[0] + (buffer[1] << 8);
}

inline void Uint16ToBuffer(uint16 value, byte buffer[2]) {
	buffer[0] = (value & 0x00FF);
	buffer[1] = (value & 0xFF00);
}
