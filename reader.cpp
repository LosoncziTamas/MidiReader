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

struct __attribute__((packed)) MidiHeaderChunk
{
	char headerType[4];
	Int32 length;
	Int16 format;
	Int16 trackChunkCount;
	Int16 division;
};

struct __attribute__((packed)) MidiTrackChunk
{
	char headerType[4];
	Int32 length;
	unsigned char delta;
};

Int32 ReverseByteOrderInt32(Int32 data)
{
	Int32 result = {0};

#if __has_builtin(__builtin_bswap32)
	result.integer = __builtin_bswap32(data.integer);
#else
	result.bytes[0] = data.bytes[3];
	result.bytes[1] = data.bytes[2];
	result.bytes[2] = data.bytes[1];
	result.bytes[3] = data.bytes[0];
#endif

	return result;
}

Int16 ReverseByteOrderInt16(Int16 data)
{
	Int16 result = {0};

#if __has_builtin(__builtin_bswap32)
	result.integer = __builtin_bswap16(data.integer);
#else
	result.bytes[0] = data.bytes[1];
	result.bytes[1] = data.bytes[0];
#endif

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
	MidiHeaderChunk *header = reinterpret_cast<MidiHeaderChunk*>(fileBuffer);
	MidiTrackChunk *track = reinterpret_cast<MidiTrackChunk*>(fileBuffer + sizeof(MidiHeaderChunk));

	auto reversedLen = ReverseByteOrderInt32(header->length);
	auto format = static_cast<Format>(ReverseByteOrderInt16(header->format).integer);
	auto trackChunkCount = ReverseByteOrderInt16(header->trackChunkCount);
	auto division = header->division;

	printf("length %d \n", reversedLen.integer);
	printf("format %d \n", format);
	printf("trackChunkCount %d \n", trackChunkCount.integer);

	if (IsSet(division, SecondSubdivision))
	{
		auto smpte = GetMaskValue(division, SMPTE);
		auto ticksPerFrame = GetMaskValue(division, TicksPerFrame);
	}
	else
	{
		auto ticksPerNote = GetMaskValue(division, TicksPerQuarterNote);
	}

	if (format == SimultaneousTracks)
	{
		// a header chunk followed by one or more track chunks
		// the tempo map must be stored as the first track
		// All MIDI Files should specify tempo and time signature
		// In format 1, these meta-events should be contained in the first track.
	}

	for (auto i = 0; i < readByteCount; ++i)
	{
		printf("%c", fileBuffer[i]);
	}

	fclose(file);
}