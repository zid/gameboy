#ifndef MBC_H
#define MBC_H
unsigned int MBC1_write_byte(unsigned short, unsigned char);
unsigned int MBC3_write_byte(unsigned short, unsigned char);
void external_ram_init(const char*);
unsigned char mbc_get_byte(unsigned short);
#endif
