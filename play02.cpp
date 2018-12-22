//============================================================================
// Name        : play02.cpp
// Author      : Mark Germain
// Version     :
// Copyright   : Copyright 2017 Germain Consulting
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

using namespace std;


constexpr uint8_t NUM_UINT_64 = 2;
constexpr uint8_t NUM_BYTES = NUM_UINT_64 * 8;
using byteArray_t = uint8_t[NUM_BYTES];

union Slider {
	byteArray_t byteAra;
	uint64_t big[NUM_UINT_64] = {0};
};

/// take a uint64_t and then mask and shift bits to get pattern we want in lowest 7 bytes
/// @param input what we want to transform
/// @return result of mask and shift
uint64_t compressUint64(uint64_t input) {
	// little endian means big arranged as bytes[7]bytes[6].....bytes[1]bytes[0]
	constexpr uint64_t mask0 = 0x7F;        // 7 bit for lowest byte
	constexpr uint64_t mask1 = mask0 << 8;  // 7 bits for next byte
	constexpr uint64_t mask2 = mask1 << 8;  // 7 bits for next byte
	constexpr uint64_t mask3 = mask2 << 8;  // 7 bits for next byte
	constexpr uint64_t mask4 = mask3 << 8;  // 7 bits for next byte
	constexpr uint64_t mask5 = mask4 << 8;  // 7 bits for next byte
	constexpr uint64_t mask6 = mask5 << 8;  // 7 bits for next byte
	constexpr uint64_t mask7 = mask6 << 8;  // 7 bits for next byte

	constexpr uint8_t shift1 = 1;  // shift byte right to over write leading zeros in next byte
	constexpr uint8_t shift2 = 2;  // shift byte right to over write leading zeros in next byte
	constexpr uint8_t shift3 = 3;  // shift byte right to over write leading zeros in next byte
	constexpr uint8_t shift4 = 4;  // shift byte right to over write leading zeros in next byte
	constexpr uint8_t shift5 = 5;  // shift byte right to over write leading zeros in next byte
	constexpr uint8_t shift6 = 6;  // shift byte right to over write leading zeros in next byte
	constexpr uint8_t shift7 = 7;  // shift byte right to over write leading zeros in next byte

	return (input & mask0)             // lowest byte
		 | (input & mask1) >> shift1   // next byte shifted right
		 | (input & mask2) >> shift2   // next byte shifted right
		 | (input & mask3) >> shift3   // next byte shifted right
		 | (input & mask4) >> shift4   // next byte shifted right
		 | (input & mask5) >> shift5   // next byte shifted right
		 | (input & mask6) >> shift6   // next byte shifted right
		 | (input & mask7) >> shift7;  // next byte shifted right
}

/// take an array of bytes which has been grouped to have every 7th byte empty
/// compress the seven bytes block to the right by the correct amount to fill in the gaps
/// @param dataP the start of the array (in little endian order) which will be squeezed in place
/// @param size length of  the array, expected to be a multiple of 8
void sqeezeSevens(uint8_t *dataP, const uint8_t size) {
	uint8_t *const endP = dataP + size;
	uint8_t *destP = dataP + 7;  // first empty byte
	uint8_t *srcP = destP + 1;   // first block of 7 bytes to move

	uint8_t count = 7;
	while (srcP < endP) {
		*destP++ = *srcP++;
		if (--count == 0) {
			++srcP;  // step over the next empty byte
			count = 7;
		}
	}
	memset(destP, 0x00, endP - destP);  // clear remaining bytes in the array
}


std::ostream& operator<<(std::ostream& os, const byteArray_t &valP) {
	os << "{";
	const char* sepStr = "";
	for (int idx = NUM_BYTES - 1; idx >= 0; --idx ) {
		uint16_t val = valP[idx];
		os << sepStr
		    << std::hex
			<< std::setw(2)
		    << std::setfill('0')
		    << val;
		sepStr = ", ";
	}
	os << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Slider& slider) {
	os << slider.byteAra;
	return os;
}

int main() {

	// set up expected result
	constexpr uint8_t expectCycle[7] = {0xc1, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x83};
	uint8_t idxSrc = 0;
	uint8_t idx = 0;
	byteArray_t expected = {0};
	while (idx < (NUM_BYTES - NUM_UINT_64)) {
		expected[idx++] = expectCycle[idxSrc++];
		if (idxSrc == sizeof(expectCycle)) { idxSrc = 0; }
	}

	Slider slider;

	std::memset(slider.byteAra, 0x41, NUM_BYTES);
	std::cout << "   start: " << slider << std::endl;

	for (uint8_t idx = 0; idx < NUM_UINT_64; ++idx) {
		slider.big[idx] = compressUint64(slider.big[idx]);
	}
	sqeezeSevens(slider.byteAra, NUM_BYTES);

	auto res = std::memcmp(expected, slider.byteAra, sizeof(expected));
	if (0 == res) {
		std::cout << " success: " << slider << std::endl;
	} else {
		std::cout << "expected: " << expected << std::endl;
		std::cout << "  actual: " << slider << std::endl;
	}
	return res;
}
