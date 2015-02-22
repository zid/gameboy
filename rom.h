#ifndef ROM_H
#define ROM_H
int rom_load(const char *);
unsigned char *rom_getbytes(void);
unsigned int rom_get_mapper(void);
int get_ram_size(void);
int has_battery(void);
enum {
	NROM,
	MBC1,
	MBC2,
	MMM01,
	MBC3,
	MBC4,
	MBC5,
};
#endif
