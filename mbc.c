#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#endif
#include <stdlib.h>
#include "mbc.h"
#include "mem.h"
#include "rom.h"

enum {
	NO_FILTER_WRITE,
	FILTER_WRITE
};

static unsigned int bank_upper_bits;
static unsigned int ram_select;
static unsigned int ram_enabled;
static unsigned char current_ram_bank;

unsigned char *ram;

#ifdef _WIN32
BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
#endif

void external_ram_init(const char* filename)
{
	if (get_ram_size() != 0)
	{
		ram = calloc(1, get_ram_size());
		current_ram_bank = 0;

		if (has_battery())
		{
#ifdef _WIN32
			HANDLE f, map;

			char sav[100];
			strcpy(sav, filename);
			strcat(sav, ".sav");

			if (FileExists(sav))
			{

				f = CreateFile(sav, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (f == INVALID_HANDLE_VALUE)
				{
					return;
				}

			}
			else
			{
				f = CreateFile(sav, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if (f == INVALID_HANDLE_VALUE)
				{
					return;
				}

				WriteFile(f, ram, get_ram_size(), NULL, NULL);
			}


			map = CreateFileMapping(f, NULL, PAGE_READWRITE, 0, 0, NULL);
			if (!map)
			{
				return;
			}

			ram = MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (!ram)
			{
				return;
			}

#endif
		}
	}
}

unsigned char mbc_get_byte(unsigned short d) // Read external RAM if any
{
	if (get_ram_size() != 0 && ram_enabled)
		return ram[current_ram_bank * 0x2000 + d - 0xA000];
	else
		return 0;
}

/* Unfinished, no clock etc */
unsigned int MBC3_write_byte(unsigned short d, unsigned char i)
{
	int bank;

	if(d < 0x2000)
	{
		if ((i & 0xFF) == 0x0A)
		{
			ram_enabled = 0xFF;
		}
		else if ((i & 0xFF) == 0x00)
		{
			ram_enabled = 0x00;
		}
		return FILTER_WRITE;
	}

	if(d < 0x4000)
	{
		bank = i & 0x7F;

		if(bank == 0)
			bank++;

		mem_bank_switch(bank);

		return FILTER_WRITE;
	}
	if (d < 0x6000)
	{

		if (i >= 0x00 && i <= 0x03) //maps the corresponding external RAM Bank (if any) into memory at A000-BFFF
		{
			current_ram_bank = i;
		}
		else if (i >= 0x08 && i <= 0x0C) //map the corresponding RTC register into memory at A000 - BFFF
		{
			printf("RTC register : %d\n", i); // TODO : RTC
		}

		return FILTER_WRITE;
	}
	if(d < 0x8000)
		return FILTER_WRITE; // TODO : RTC

	if (d >= 0xA000 && d < 0xBFFF && ram_enabled)
	{
		ram[current_ram_bank * 0x2000 + d - 0xA000] = i;

		return FILTER_WRITE;
	}

	return NO_FILTER_WRITE;
}
unsigned int MBC1_write_byte(unsigned short d, unsigned char i)
{
	int bank;

	if(d < 0x2000)
	{
		if ((i & 0xFF) == 0x00)
			ram_enabled = 0x00;
		else if ((i & 0xFF) == 0x0A)
			ram_enabled = 0xFF;

		return FILTER_WRITE;
	}

	/* Switch rom bank at 4000-7fff */
	if(d >= 0x2000 && d < 0x4000)
	{
		/* Bits 0-4 come from the value written to memory here,
		 * bits 5-6 come from a seperate write to 4000-5fff if
		 * RAM select is 1.
		 */
		bank = i & 0x1F;
		if(!ram_select)
			bank |= bank_upper_bits;

		/* "Writing to this address space selects the lower 5 bits of the
		 * ROM Bank Number (in range 01-1Fh). When 00h is written, the MBC
		 * translates that to bank 01h also."
		 * http://nocash.emubase.de/pandocs.htm#mbc1max2mbyteromandor32kbyteram
		 */

		if(bank == 0 || bank == 0x20 || bank == 0x40 || bank == 0x60)
			bank++;

		mem_bank_switch(bank);

		return FILTER_WRITE;
	}

	/* Bit 5 and 6 of the bank selection */
	if(d >= 0x4000 && d < 0x6000)
	{
		if (!ram_select)
			bank_upper_bits = (i & 0x3) << 5;
		else
			current_ram_bank = i & 0xFF;
		return FILTER_WRITE;
	}

	if(d >= 0x6000 && d <= 0x7FFF)
	{
		ram_select = i&1;
		return FILTER_WRITE;
	}
	return NO_FILTER_WRITE;
}
