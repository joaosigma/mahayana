#include "ifHDRI.hpp"

#include "common\imageFactory.hpp"

#define SCANLINE_MINELEN	8				// minimum scanline length for encoding
#define SCANLINE_MAXELEN	0x7fff			// maximum scanline length for encoding

typedef unsigned char RGBE[4];

static
void workOnRGBE(const RGBE *scan, int len, float *cols)
{
	double auxD;

	for (; len-- > 0; cols += 4, scan++)
	{
		if (scan[0][3] == 0)
		{
			cols[0] = cols[1] = cols[2] = 0.0f;
			cols[3] = 1.0f;
			continue;
		}

		auxD = pow(2.0, ((int)scan[0][3]) - (128 + 8));

		cols[0] = (float)(auxD*(((double)scan[0][0]) + 0.5));
		cols[1] = (float)(auxD*(((double)scan[0][1]) + 0.5));
		cols[2] = (float)(auxD*(((double)scan[0][2]) + 0.5));
		cols[3] = 1.0f;
	}
}

static
bool oldDecrunch(RGBE *scanline, int len, HorseRadish::Streams::StreamReader &streamReader)
{
	int i, rshift;

	rshift = 0;
	while (len > 0)
	{
		if (streamReader.Read(&scanline[0][0], 4) != 4)
			return false;

		if (scanline[0][0] == 1 && scanline[0][1] == 1 && scanline[0][2] == 1)
		{
			for (i = scanline[0][3] << rshift; i > 0; i--)
			{
				memcpy(&scanline[0][0], &scanline[-1][0], 4);
				scanline++;
				len--;
			}

			rshift += 8;
			continue;
		}

		scanline++;
		len--;
		rshift = 0;
	}

	return true;
}

static
bool decrunch(RGBE *scanline, int len, HorseRadish::Streams::StreamReader &streamReader)
{
	int i;
	unsigned char code, val;

	if (len < SCANLINE_MINELEN || len > SCANLINE_MAXELEN)
		return oldDecrunch(scanline, len, streamReader);

	i = 0;
	streamReader.Read(&i, 1);

	if (i != 2)
	{
		streamReader.Seek(-1);
		return oldDecrunch(scanline, len, streamReader);
	}

	streamReader.Read(&scanline[0][1], 2);

	i = 0;
	streamReader.Read(&i, 1);

	if (scanline[0][1] != 2 || scanline[0][2] & 128)
	{
		scanline[0][0] = 2;
		scanline[0][3] = i;
		return oldDecrunch(scanline + 1, len - 1, streamReader);
	}

	// read each component
	for (i = 0; i < 4; i++)
	{
		for (int j = 0; j < len; )
		{
			streamReader.Read(&code, 1);

			if (code > 128)
			{
				code &= 127;
				streamReader.Read(&val, 1);

				while (code--)
					scanline[j++][i] = val;
				continue;
			}

			while (code--)
				streamReader.Read(&scanline[j++][i], 1);
		}
	}

	return true;
}

#undef SCANLINE_MINELEN
#undef SCANLINE_MAXELEN

HorseRadish::Imaging::Image* readHDRI(HorseRadish::Streams::StreamReader &streamReader)
{
	int i, w, h;
	char str[200], c, oldc;
	RGBE *scanline;
	float *imgWalker;
	HorseRadish::Imaging::Image *newImage;

	c = 0;
	while (true)
	{
		oldc = c;
		if (streamReader.Read(&c, 1) != 1)
			return nullptr;
		if (c == 0xa && oldc == 0xa)
			break;
	}

	i = 0;
	while (true)
	{
		if (streamReader.Read(&c, 1) != 1)
			return nullptr;

		str[i++] = c;
		str[i] = '\0';

		if (c == 0xa)
			break;
		if (i >= 199)
			break;
	}

	if (sscanf_s(str, "-Y %ld +X %ld", &h, &w) != 2)
		return nullptr;

	newImage = HorseRadish::Imaging::Factory::CreateImageFloat(w, h, HorseRadish::Imaging::Image::RGBA, &imgWalker);
	if (newImage == nullptr)
		return nullptr;

	scanline = (RGBE*)malloc(sizeof(RGBE)*w);
	if (!scanline)
	{
		delete newImage;
		return nullptr;
	}

	for (i = h - 1; i >= 0; i--)
	{
		if (decrunch(scanline, w, streamReader) == false)
			break;
		workOnRGBE(scanline, w, imgWalker + i*w * 4);
	}

	free(scanline);

	return newImage;
}