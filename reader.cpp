#include <stdio.h>
#include <assert.h>

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

char fileBuffer[1024] = {0};

enum Format
{
	SingleTrack = 0,
	SimultaneousTracks = 1,
	SingleTrackPatterns = 2
};

enum DivisionMask
{
	SecondSubdivision = 0x8000,
	SMPTE = 0x7F00,
	TicksPerFrame = 0xFF,
	TicksPerQuarterNote = 0x7FFF,
};

union Int32
{
	unsigned char bytes[4];
	unsigned int integer;
};

union Int16
{
	unsigned char bytes[2];
	unsigned short integer;
};

struct __attribute__((packed)) Midi
{
	char headerType[4];
	Int32 length;
	Int16 format;
	Int16 trackChunkCount;
	Int16 division;
};

Int32 ReverseByteOrderInt32(Int32 data)
{
	Int32 result = {0};

	result.bytes[0] = data.bytes[3];
	result.bytes[1] = data.bytes[2];
	result.bytes[2] = data.bytes[1];
	result.bytes[3] = data.bytes[0];

	return result;
}

Int16 ReverseByteOrderInt16(Int16 data)
{
	Int16 result = {0};

	result.bytes[0] = data.bytes[1];
	result.bytes[1] = data.bytes[0];

	return result;
}

unsigned int GetMaskValue(Int16 bits, DivisionMask mask)
{
	return bits.integer & mask;
}

bool IsSet(Int16 bits, DivisionMask mask)
{
	return GetMaskValue(bits, mask) != 0;
}

int main(int argc, char** argv)
{
	auto* fileName = "C-natural_major.mid";
	auto* file = fopen(fileName, "rb");
	fseek(file, 0, SEEK_END);
    auto byteCount = ftell(file);
	rewind(file);

	assert(byteCount < ArrayCount(fileBuffer));

	auto readByteCount = fread(fileBuffer, sizeof(char), byteCount, file);

	assert(readByteCount == byteCount);
	Midi *midi = reinterpret_cast<Midi*>(fileBuffer);

	auto reversedLen = ReverseByteOrderInt32(midi->length);
	auto format = static_cast<Format>(ReverseByteOrderInt16(midi->format).integer);
	auto trackChunkCount = ReverseByteOrderInt16(midi->trackChunkCount);
	auto division = midi->division;

	if (IsSet(division, SecondSubdivision))
	{
		auto smpte = GetMaskValue(division, SMPTE);
		auto ticksPerFrame = GetMaskValue(division, TicksPerFrame);
	}
	else
	{
		auto ticksPerNote = GetMaskValue(division, TicksPerQuarterNote);
	}

	printf("%s \n", midi->headerType);
	printf("%u \n", midi->length.bytes[0]);
	printf("%u \n", midi->length.bytes[1]);
	printf("%u \n", midi->length.bytes[2]);
	printf("%u \n", midi->length.bytes[3]);

	for (auto i = 0; i < readByteCount; ++i)
	{
		printf("%c", fileBuffer[i]);
	}

	fclose(file);
}