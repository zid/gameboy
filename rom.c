#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include "rom.h"

unsigned char *bytes;
unsigned int mapper;

static char *carts[] = {
	"ROM ONLY",
	"MBC1",
	"MBC1+RAM",
	"MBC1+RAM+BATTERY",
	"MBC2",
	"MBC2+BATTERY",
	"ROM+RAM",
	"ROM+RAM+BATTERY",
	"MMM01",
	"MMM01+RAM",
	"MMM01+RAM+BATTERY",
	"MBC3+TIMER+BATTERY",
	"MBC3+TIMER+RAM+BATTERY",
	"MBC3",
	"MBC3+RAM",
	"MBC3+RAM+BATTERY",
	"MBC4",
	"MBC4+RAM",
	"MBC4+RAM+BATTERY",
	"MBC5",
	"MBC5+RAM",
	"MBC5+RAM+BATTERY",
	"MBC5+RUMBLE",
	"MBC5+RUMBLE+RAM",
	"MBC5+RUMBLE+RAM+BATTERY",
	/* 0xFC */
	"POCKET CAMERA",
	"BANDAI TAMA5",
	"HuC3",
	"HuC1+RAM+BATTERY",
	"Unknown"
};

static char *banks[] = {
	" 32KiB",
	" 64KiB",
	"128KiB",
	"256KiB",
	"512KiB",
	"  1MiB",
	"  2MiB",
	"  4MiB",
	/* 0x52 */
	"1.1MiB",
	"1.2MiB",
	"1.5MiB",
	"Unknown"
};

static char *rams[] = {
	"None",
	"  2KiB",
	"  8KiB",
	" 32KiB",
	"Unknown"
};

static char *regions[] = {
	"Japan",
	"Non-Japan",
	"Unknown"
};

static unsigned char header[] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

static int rom_init(unsigned char *rombytes)
{
	char buf[17];
	int type, bank_index, ram, region, version, i, pass;
	unsigned char checksum = 0;

	if(memcmp(&rombytes[0x104], header, sizeof(header)) != 0)
		return 0;

	memcpy(buf, &rombytes[0x134], 16);
	buf[16] = '\0';
	printf("Rom title: %s\n", buf);

	type = rombytes[0x147];

	/* There's a gap in the cartridge table between 0x1E and 0xFC
	 * but the array does not have this gap to save space. Adjust
	 * the type number so that it aligns to the array.
	 */
	if(type >= 0xFC)
		type = type - 227;
	else if(type > 0x1E)
		type = 29;

	printf("Cartridge type: %s\n", carts[type]);

	bank_index = rombytes[0x148];
	/* Adjust for the gap in the bank indicies */
	if(bank_index >= 0x52 && bank_index <= 0x54)
		bank_index -= 74;
	else if(bank_index > 7)
		bank_index = 11;

	printf("Rom size: %s\n", banks[bank_index]);

	ram = rombytes[0x149];
	if(ram > 3)
		ram = 4;

	printf("RAM size: %s\n", rams[ram]);

	region = rombytes[0x14A];
	if(region > 2)
		region = 2;
	printf("Region: %s\n", regions[region]);

	version = rombytes[0x14C];
	printf("Version: %02X\n", version);

	for(i = 0x134; i <= 0x14C; i++)
		checksum = checksum - rombytes[i] - 1;

	pass = rombytes[0x14D] == checksum;

	printf("Checksum: %s (%02X)\n", pass ? "OK" : "FAIL", checksum);
	if(!pass)
		return 0;

	bytes = rombytes;

	switch(type)
	{
		case 0x00:
		case 0x08:
		case 0x09:
			mapper = NROM;
		break;
		case 0x01:
		case 0x02:
		case 0x03:
			mapper = MBC1;
		break;
		case 0x05:
		case 0x06:
			mapper = MBC2;
		break;
		case 0x0B:
		case 0x0C:
			mapper = MMM01;
		break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			mapper = MBC3;
		break;
		case 0x15:
		case 0x16:
		case 0x17:
			mapper = MBC4;
		break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			mapper = MBC5;
		break;
	}

	return 1;
}

unsigned int rom_get_mapper(void)
{
	return mapper;
}

int rom_load(const char *filename)
{
#ifdef _WIN32
	HANDLE f, map;
#else
	int f;
	size_t length;
	struct stat st;
#endif
	unsigned char *bytes;

#ifdef _WIN32
	f = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(f == INVALID_HANDLE_VALUE)
		return 0;

	map = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
	if(!map)
		return 0;

	bytes = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
	if(!bytes)
		return 0;
#else
	f = open(filename, O_RDONLY);
	if(f == -1)
		return 0;
	if(fstat(f, &st) == -1)
		return 0;

	bytes = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f, 0);
	if(!bytes)
		return 0;
#endif
	return rom_init(bytes);
}

unsigned char *rom_getbytes(void)
{
	return bytes;
}
