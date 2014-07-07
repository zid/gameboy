#include <stdio.h>
#include "mem.h"
#include "rom.h"
#include "interrupt.h"

#define set_HL(x) c.L = (x)&0xFF; c.H = (x)>>8
#define set_BC(x) c.C = (x)&0xFF; c.B = (x)>>8
#define set_DE(x) c.E = (x)&0xFF; c.D = (x)>>8
#define set_AF(x) c.F = ((x)&0xFF); c.A = ((x)>>8)

#define get_AF() ((c.A<<8) | c.F)
#define get_BC() ((c.B<<8) | c.C)
#define get_DE() ((c.D<<8) | c.E)
#define get_HL() ((c.H<<8) | c.L)

/* Flags */
#define set_Z(x) c.F = ((c.F&0x7F) | ((x)<<7))
#define set_N(x) c.F = ((c.F&0xBF) | ((x)<<6))
#define set_H(x) c.F = ((c.F&0xDF) | ((x)<<5))
#define set_C(x) c.F = ((c.F&0xEF) | ((x)<<4))

#define flag_Z !!((c.F & 0x80))
#define flag_C !!((c.F & 0x10))

struct CPU {
	unsigned char H;
	unsigned char L;

	unsigned char D;
	unsigned char E;

	unsigned char B;
	unsigned char C;

	unsigned char A;
	unsigned char F;

	unsigned short SP;
	unsigned short PC;
	unsigned int cycles;
};

static struct CPU c;
static int is_debugged;
static int halted;

void cpu_init(void)
{
	set_AF(0x01B0);
	set_BC(0x0013);
	set_DE(0x00D8);
	set_HL(0x014D);
	c.SP = 0xFFFE;
	c.PC = 0x0100;
	c.cycles = 0;
}

static void RL(unsigned int reg)
{
	unsigned char t, t2;

	switch(reg)
	{
		case 0: /* B */
			t2 = flag_C;
			set_C(!!(c.B&0x80));
			c.B = (c.B << 1) | !!(t2);
		break;
		case 1: /* C */
			t2 = flag_C;
			set_C(!!(c.C&0x80));
			c.C = (c.C << 1) | !!(t2);
		break;
		case 2: /* D */
			t2 = flag_C;
			set_C(!!(c.D&0x80));
			c.D = (c.D << 1) | !!(t2);
		break;
		case 3: /* E */
			t2 = flag_C;
			set_C(!!(c.E&0x80));
			c.E = (c.E << 1) | !!(t2);
		break;
		case 4: /* H */
			t2 = flag_C;
			set_C(!!(c.H&0x80));
			c.H = (c.H << 1) | !!(t2);
		break;
		case 5: /* L */
			t2 = flag_C;
			set_C(!!(c.H&0x80));
			c.H = (c.H << 1) | !!(t2);
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			t2 = flag_C;
			set_C(!!(t&0x80));
			t = (t << 1) | !!(t2);
			mem_write_byte(get_HL(), t);
			c.cycles += 2;
		break;
		case 7: /* A */
			t2 = flag_C;
			set_C(!!(c.A&0x80));
			c.A = (c.A << 1) | !!(t2);
		break;
	}

	c.cycles += 2;
}

static void RR(unsigned int reg)
{
	unsigned char t, t2;

	switch(reg)
	{
		case 0: /* B */
			t2 = flag_C;
			set_C(c.B&1);
			c.B = (c.B >> 1) | (!!t2)<<7;
		break;
		case 1: /* C */
			t2 = flag_C;
			set_C(c.C&1);
			c.C = (c.C >> 1) | (!!t2)<<7;
		break;
		case 2: /* D */
			t2 = flag_C;
			set_C(c.D&1);
			c.D = (c.D >> 1) | (!!t2)<<7;
		break;
		case 3: /* E */
			t2 = flag_C;
			set_C(c.E&1);
			c.E = (c.E >> 1) | (!!t2)<<7;
		break;
		case 4: /* H */
			t2 = flag_C;
			set_C(c.H&1);
			c.H = (c.H >> 1) | (!!t2)<<7;
		break;
		case 5: /* L */
			t2 = flag_C;
			set_C(c.H&1);
			c.H = (c.H >> 1) | (!!t2)<<7;
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			t2 = flag_C;
			set_C(t&1);
			t = (t >> 1) | (!!t2)<<7;
			mem_write_byte(get_HL(), t);
			c.cycles += 2;
		break;
		case 7: /* A */
			t2 = flag_C;
			set_C(c.A&1);
			c.A = (c.A >> 1) | (!!t2)<<7;
		break;
	}

	c.cycles += 2;
}

static void SLA(unsigned int reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			set_C(!!(c.B & 0x80));
			c.B = c.B << 1;
			set_Z(!c.B);
		break;
		case 1: /* C */
			set_C(!!(c.C & 0x80));
			c.C = c.C << 1;
			set_Z(!c.C);
		break;
		case 2: /* D */
			set_C(!!(c.D & 0x80));
			c.D = c.D << 1;
			set_Z(!c.D);
		break;
		case 3: /* E */
			set_C(!!(c.E & 0x80));
			c.E = c.E << 1;
			set_Z(!c.E);
		break;
		case 4: /* H */
			set_C(!!(c.H & 0x80));
			c.H = c.H << 1;
			set_Z(!c.H);
		break;
		case 5: /* L */
			set_C(!!(c.L & 0x80));
			c.L = c.L << 1;
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			set_C(!!(t & 0x80));
			t = t << 1;
			mem_write_byte(get_HL(), t);
			set_Z(!t);
			c.cycles += 2;
		break;
		case 7: /* A */
			set_C(!!(c.A & 0x80));
			c.A = c.A << 1;
			set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);

	c.cycles += 2;
}

static void SLR(unsigned int reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			set_C(c.B & 1);
			c.B = c.B >> 1;
			set_Z(!c.B);
		break;
		case 1: /* C */
			set_C(c.C & 1);
			c.C = c.C >> 1;
			set_Z(!c.C);
		break;
		case 2: /* D */
			set_C(c.D & 1);
			c.D = c.D >> 1;
			set_Z(!c.D);
		break;
		case 3: /* E */
			set_C(c.E & 1);
			c.E = c.E >> 1;
			set_Z(!c.E);
		break;
		case 4: /* H */
			set_C(c.H & 1);
			c.H = c.H >> 1;
			set_Z(!c.H);
		break;
		case 5: /* L */
			set_C(c.L & 1);
			c.L = c.L >> 1;
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			set_C(t & 1);
			t = t >> 1;
			mem_write_byte(get_HL(), t);
			set_Z(!t);
			c.cycles += 2;
		break;
		case 7: /* A */
			set_C(c.A & 1);
			c.A = c.A >> 1;
			set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);

	c.cycles += 2;
}

static void SWAP(unsigned int reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			c.B = ((c.B&0xF)<<4) | ((c.B&0xF0)>>4);
			c.F = (!c.B)<<7;
		break;
		case 1: /* C */
			c.C = ((c.C&0xF)<<4) | ((c.C&0xF0)>>4);
			c.F = (!c.C)<<7;
		break;
		case 2: /* D */
			c.D = ((c.D&0xF)<<4) | ((c.D&0xF0)>>4);
			c.F = (!c.D)<<7;
		break;
		case 3: /* E */
			c.E = ((c.E&0xF)<<4) | ((c.E&0xF0)>>4);
			c.F = (!c.E)<<7;
		break;
		case 4: /* H */
			c.H = ((c.H&0xF)<<4) | ((c.H&0xF0)>>4);
			c.F = (!c.H)<<7;
		break;
		case 5: /* L */
			c.L = ((c.L&0xF)<<4) | ((c.L&0xF0)>>4);
			c.F = (!c.L)<<7;
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			t = ((t&0xF)<<4) | ((t&0xF0)>>4);
			mem_write_byte(get_HL(), t);
			c.F = (!t)<<7;
			c.cycles += 2;
		break;
		case 7: /* A */
			c.A = ((c.A&0xF)<<4) | ((c.A&0xF0)>>4);
			c.F = (!c.A)<<7;
		break;
	}
	c.cycles += 2;
}

static void BIT(unsigned int bit, unsigned int reg)
{
	unsigned char t, f = 0 /* Make GCC happy */;

	switch(reg)
	{
		case 0: /* B */
		    f = !!(c.B & bit);
		break;
		case 1: /* C */
		    f = !!(c.C & bit);
		break;
		case 2: /* D */
		    f = !!(c.D & bit);
		break;
		case 3: /* E */
		    f = !!(c.E & bit);
		break;
		case 4: /* H */
		    f = !!(c.H & bit);
		break;
		case 5: /* L */
		    f = !!(c.L & bit);
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			f = !!(t & bit);
			c.cycles += 1;
		break;
		case 7: /* A */
		    f = !!(c.A & bit);
		break;
	}

	set_Z(f);
	set_N(0);
	set_H(1);

	c.cycles += 2;
}

static void RES(unsigned int bit, unsigned int reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			c.B ^= bit;
		break;
		case 1: /* C */
			c.C ^= bit;
		break;
		case 2: /* D */
			c.D ^= bit;
		break;
		case 3: /* E */
			c.E ^= bit;
		break;
		case 4: /* H */
			c.H ^= bit;
		break;
		case 5: /* L */
			c.L ^= bit;
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			t ^= bit;
			mem_write_byte(get_HL(), t);
			c.cycles += 2;
		break;
		case 7: /* A */
			c.A ^= bit;
		break;
	}
	c.cycles += 2;
}

static void SET(unsigned int bit, unsigned int reg)
{
	unsigned char t;

	switch(reg)
	{

		case 0: /* B */
			c.B |= bit;
		break;
		case 1: /* C */
			c.C |= bit;
		break;
		case 2: /* D */
			c.D |= bit;
		break;
		case 3: /* E */
			c.E |= bit;
		break;
		case 4: /* H */
			c.H |= bit;
		break;
		case 5: /* L */
			c.L |= bit;
		break;
		case 6: /* (HL) */
			t = mem_get_byte(get_HL());
			t |= bit;
			mem_write_byte(get_HL(), t);
			c.cycles += 2;
		break;
		case 7: /* A */
			c.A |= bit;
		break;
	}
	c.cycles += 2;
}

void cpu_interrupt(unsigned short vector)
{
	halted = 0;

	c.SP -= 2;
	mem_write_word(c.SP, c.PC);
	c.PC = vector;
	interrupt_disable();
}

unsigned int cpu_get_cycles(void)
{
	return c.cycles;
}

int cpu_cycle(void)
{
	unsigned char b, t;
	unsigned short s;
	unsigned int i;

	if(halted)
	{
		c.cycles += 1;
		return 1;
	}

	b = mem_get_byte(c.PC);

	if(c.PC == 0x100)
		is_debugged = 1;

	if(is_debugged)
	{
		printf("%04X: %02X\n", c.PC, b);
		printf("\tAF: %02X%02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X SP: %04X\n",
			c.A, c.F, c.B, c.C, c.D, c.E, c.H, c.L, c.SP);
	}

	switch(b)
	{
		case 0x00:  /* NOP */
			c.PC++;
			c.cycles += 1;
		break;
		case 0x01:  /* LD BC, imm16 */
			c.C = mem_get_byte(c.PC+1);
			c.B = mem_get_byte(c.PC+2);
			c.PC += 3;
			c.cycles += 3;
		break;
		case 0x03:  /* INC BC */
			s = get_BC();
			s++;
			set_BC(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x04:  /* INC B */
			c.B++;
			set_Z(!c.B);
			set_N(1);
			set_H(c.B == 0x10);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x05:  /* DEC B */
			c.B--;
			set_Z(!c.B);
			set_N(1);
			set_H((c.B & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x06:  /* LD B, imm8 */
			c.B = mem_get_byte(c.PC+1);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0x0B:  /* DEC BC */
			s = get_BC();
			s--;
			set_BC(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x0C:  /* INC C */
			c.C++;
			set_Z(!c.C);
			set_N(1);
			set_H(c.C == 0x10);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x0D:  /* DEC C */
			c.C--;
			set_Z(!c.C);
			set_N(1);
			set_H((c.C & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x0E:  /* LD C, imm8 */
			c.C = mem_get_byte(c.PC+1);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0x0F:  /* RRCA */
			set_C(c.A&1);
			c.A = c.A >> 1;
			c.A |= flag_C<<7;
			set_H(0);
			set_N(0);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x11:  /* LD DE, imm16 */
			s = mem_get_word(c.PC+1);
			set_DE(s);
			c.PC += 3;
			c.cycles += 3;
		break;
		case 0x12:  /* LD (DE), A */
			mem_write_byte(get_DE(), c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x13:  /* INC DE */
			s = get_DE();
			s++;
			set_DE(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x14:  /* INC D */
			c.D++;
			set_Z(!c.D);
			set_H(c.D == 0x10);
			set_N(0);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x16:  /* LD D, imm8 */
			c.D = mem_get_byte(c.PC+1);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x18:  /* JR rel8 */
			c.PC += (signed char)mem_get_byte(c.PC+1) + 2;
			c.cycles += 3;
		break;
		case 0x19:  /* ADD HL, DE */
			i = get_HL();
			i += get_DE();
			set_H(get_HL() < 0x800 && i >= 0x800);
			set_HL(i & 0xFFFF);
			set_N(0);
			set_C(i > 0xFFFF);
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0x1A:  /* LD A, (DE) */
			c.A = mem_get_byte(get_DE());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x1B:  /* DEC DE */
			s = get_DE();
			s--;
			set_DE(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x1C:  /* INC E */
			c.E++;
			set_Z(!c.E);
			set_H(c.E == 0x10);
			set_N(0);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x1D:  /* DEC E */
			c.E--;
			set_Z(!c.E);
			set_N(1);
			set_H((c.E & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x1E:  /* LD E, imm8 */
			c.E = mem_get_byte(c.PC+1);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0x1F:  /* RR A */
			t = flag_C;
			set_C(c.A&1);
			c.A = (c.A >> 1) | (!!t)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x20:  /* JR NZ, rel8 */
			if(flag_Z == 0)
			{
				c.PC += (signed char)mem_get_byte(c.PC+1) + 2;
				c.cycles += 3;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0x21:  /* ld hl, imm16 */
			c.L = mem_get_byte(c.PC+1);
			c.H = mem_get_byte(c.PC+2);
			c.PC += 3;
			c.cycles += 3;
		break;
		case 0x22:  /* ldi (hl), a */
			s = get_HL();
			mem_write_byte(s, c.A);
	        s++;
			set_HL(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x23:  /* INC HL */
			s = get_HL();
			s++;
			set_HL(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x24:  /* INC H */
			c.H++;
			set_Z(!c.H);
			set_H(c.H == 0x10);
			set_N(0);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x25:  /* DEC H */
			c.H--;
			set_Z(!c.H);
			set_N(1);
			set_H((c.H & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x26:  /* LD H, imm8 */
			c.H = mem_get_byte(c.PC+1);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0x28:  /* JR Z, rel8 */
			if(flag_Z == 1)
			{
				c.PC += (signed char)mem_get_byte(c.PC+1) + 2;
				c.cycles += 3;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0x29:  /* ADD HL, HL */
			s = get_HL();
			set_C(s > s+s);
			s += s;
			set_HL(s);
			set_N(0);
			//set_H()
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x2A:  /* LDI A, (HL) */
			s = get_HL();
			c.A = mem_get_byte(s);
			set_HL(s+1);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x2C:  /* INC L */
			c.L++;
			set_Z(!c.L);
			set_N(0);
			set_H((c.L & 0xF) == 0x00);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x2D:  /* DEC L */
			c.L--;
			set_Z(!c.L);
			set_N(1);
			set_H((c.L & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x2F:  /* CPL */
			c.A = ~c.A;
			set_N(1);
			set_H(1);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x30:  /* JR NC, rel8 */
			if(flag_C == 0)
			{
				c.PC += (signed char)mem_get_byte(c.PC+1) + 2;
				c.cycles += 3;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0x31:  /* ld sp, imm16 */
			c.SP = mem_get_word(c.PC+1);
			c.PC += 3;
			c.cycles += 3;
		break;
		case 0x32:  /* LDD (HL), A */
			s = get_HL();
			mem_write_byte(s, c.A);
	        s--;
			set_HL(s);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x34:  /* INC (HL) */
			t = mem_get_byte(get_HL());
			t++;
			mem_write_byte(get_HL(), t);
			set_Z(!t);
			set_N(1);
			set_H(t & 0x10);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x35:  /* DEC (HL) */
			t = mem_get_byte(get_HL());
			mem_write_byte(get_HL(), t-1);
			set_Z(!t);
			set_N(1);
			set_H((t & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x36:  /* LD (HL), imm8 */
			t = mem_get_byte(c.PC+1);
			mem_write_byte(get_HL(), t);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0x38:  /* JR C, rel8 */
			if(flag_C == 1)
			{
				c.PC += (signed char)mem_get_byte(c.PC+1) + 2;
				c.cycles += 3;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0x3C:  /* INC A */
			c.A++;
			set_Z(!c.A);
			set_H(c.A == 0x10);
			set_N(0);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x3D:  /* DEC A */
			c.A--;
			set_Z(!c.A);
			set_N(1);
			set_H((c.A & 0xF) == 0xF);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x3E:  /* LD A, imm8 */
			c.A = mem_get_byte(c.PC+1);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0x46:  /* LD B, (HL) */
			c.B = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x47:  /* LD B, A */
			c.B = c.A;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x4E:  /* LD C, (HL) */
			c.C = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x4F:  /* LD C, A */
			c.C = c.A;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x56:  /* LD D, (HL) */
			c.D = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x57:  /* LD D, A */
			c.D = c.A;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x5E:  /* LD E, (HL) */
			c.E = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x5F:  /* LD E, A */
			c.E = c.A;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x60:  /* LD H, B */
			c.H = c.B;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x61:  /* LD H, C */
			c.H = c.C;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x62:  /* LD H, D */
			c.H = c.D;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x63:  /* LD H, E */
			c.H = c.E;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x64:  /* LD H, H */
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x65:  /* LD H, L */
			c.H = c.L;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x66:  /* LD H, (HL) */
			c.H = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x67:  /* LD H, A */
			c.H = c.A;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x68:  /* LD L, B */
			c.L = c.B;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x69:  /* LD L, C */
			c.L = c.C;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x6A:  /* LD L, D */
			c.L = c.D;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x6B:  /* LD L, E */
			c.L = c.E;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x6C:  /* LD L, H */
			c.L = c.H;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x6D:  /* LD L, L */
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x6E:  /* LD L, (HL) */
			c.L = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x6F:  /* LD L, A */
			c.L = c.A;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x70:  /* LD (HL), B */
			mem_write_byte(get_HL(), c.B);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x71:  /* LD (HL), C */
			mem_write_byte(get_HL(), c.C);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x72:  /* LD (HL), D */
			mem_write_byte(get_HL(), c.D);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x76:  /* HALT */
			halted = 1;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x77:  /* LD (HL), A */
			mem_write_byte(get_HL(), c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x78:  /* LD A, B */
			c.A = c.B;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x79:  /* LD A, C */
			c.A = c.C;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x7A:  /* LD A, D */
			c.A = c.D;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x7B:  /* LD A, E */
			c.A = c.E;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x7C:  /* LD A, H */
			c.A = c.H;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x7D:  /* LD A, L */
			c.A = c.L;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x7E:  /* LD A, (HL) */
			c.A = mem_get_byte(get_HL());
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x81:  /* ADD C */
			t = c.A;
			c.A += c.C;
			set_Z(!c.A);
			set_H((t & 0x8) && (c.A & 0x10));
			set_N(0);
			set_C(c.A < t);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x86:  /* ADD (HL) */
			t = c.A;
			c.A += mem_get_byte(get_HL());
			set_Z(!c.A);
			set_H((t & 0x8) && (c.A & 0x10));
			set_N(0);
			set_C(c.A < t);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0x87:  /* ADD A */
			set_C(c.A < (c.A + c.A));
			set_H((c.A & 0xF) + (c.A & 0xF) >= 0x10);
			c.A += c.A;
			set_N(0);
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x91:  /* SUB C */
			set_C(c.A > (c.A - c.C));
			c.A -= c.C;
			set_N(1);
			//set_H()
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xA1:  /* AND C */
			c.A &= c.C;
			c.F = (!c.A)<<7;
			set_H(1);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xA7:  /* AND A */
			set_H(1);
			set_N(0);
			set_C(0);
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xA9:  /* XOR C */
			c.A ^= c.C;
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xAE:  /* XOR (HL) */
			c.A ^= mem_get_byte(c.PC+1);
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xAF:  /* XOR A */
			c.A = 0;
			c.F = 0x80;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xB0:  /* OR B */
			c.A |= c.B;
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xB1:  /* OR C */
			c.A |= c.C;
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xB2:  /* OR D */
			c.A |= c.D;
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xB3:  /* OR E */
			c.A |= c.E;
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xB6:  /* OR (HL) */
			c.A |= mem_get_byte(get_HL());
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xB7:  /* OR A */
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xB9:  /* CP C */
			set_Z(c.A == c.C);
			set_H((c.A & 0x10) && ((c.A-c.C) & 0x8));
			set_N(1);
			set_C((c.A - c.C) > c.A);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xC1:  /* POP BC */
			s = mem_get_word(c.SP);
			set_BC(s);
			c.SP += 2;
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xC2:  /* JP NZ, mem16 */
			if(flag_Z == 0)
			{
				c.PC = mem_get_word(c.PC+1);
			} else {
				c.PC += 3;
			}
			c.cycles += 3;
		break;
		case 0xC3:  /* JP imm16 */
			c.PC = mem_get_word(c.PC+1);
			c.cycles += 4;
		break;
		case 0xC4:  /* CALL NZ, imm16 */
			if(flag_Z == 0)
			{
				c.SP -= 2;
				mem_write_word(c.SP, c.PC+3);
				c.PC = mem_get_word(c.PC+1);
				c.cycles += 6;
			} else {
				c.PC += 3;
				c.cycles += 3;
			}
		break;
		case 0xC5:  /* PUSH BC */
			c.SP -= 2;
			mem_write_word(c.SP, get_BC());
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xC6:  /* ADD A, imm8 */
			t = mem_get_byte(c.PC+1);
			set_C(c.A < (c.A + t));
			set_H((c.A & 0xF) + (t & 0xF) >= 0x10);
			c.A += t;
			set_N(0);
			set_Z(!c.A);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0xC7:  /* RST 00 */
			c.SP -= 2;
			mem_write_word(c.SP, c.PC);
			c.PC = 0;
			c.cycles += 3;
		break;
		case 0xC8:  /* RET Z */
			if(flag_Z == 1)
			{
				c.PC = mem_get_word(c.SP);
				c.SP += 2;
				c.cycles += 3;
			} else {
				c.PC += 1;
				c.cycles += 1;
			}
		break;
		case 0xC9:  /* RET */
			c.PC = mem_get_word(c.SP);
			c.SP += 2;
			c.cycles += 3;
		break;
		case 0xCA:  /* JP z, mem16 */
			if(flag_Z == 1)
			{
				c.PC = mem_get_word(c.PC+1);
			} else {
				c.PC += 3;
			}
			c.cycles += 3;
		break;
		case 0xCB:  /* RLC/RRC/RL/RR/SLA/SRA/SWAP/SRL/BIT/RES/SET */
			t = mem_get_byte(c.PC+1);

			switch(t >> 3)
			{
				case 0x02:
					RL(t&7);
				break;
				case 0x03:
					RR(t&7);
				break;
				case 0x04:
					SLA(t&7);
				break;
				case 0x06:
					SWAP(t&7);
				break;
				case 0x07:
					SLR(t&7);
				break;
				case 0x08:
					BIT(0x01, t & 7);
				break;
				case 0x09:
					BIT(0x02, t & 7);
				break;
				case 0x0A:
					BIT(0x04, t & 7);
				break;
				case 0x0B:
					BIT(0x08, t & 7);
				break;
				case 0x0C:
					BIT(0x10, t & 7);
				break;
				case 0x0D:
					BIT(0x20, t & 7);
				break;
				case 0x0E:
					BIT(0x40, t & 7);
				break;
				case 0x0F:
					BIT(0x80, t & 7);
				break;
				case 0x10:
					RES(0x01, t & 7);
				break;
				case 0x11:
					RES(0x02, t & 7);
				break;
				case 0x12:
					RES(0x04, t & 7);
				break;
				case 0x13:
					RES(0x08, t & 7);
				break;
				case 0x14:
					RES(0x10, t & 7);
				break;
				case 0x15:
					RES(0x20, t & 7);
				break;
				case 0x16:
					RES(0x40, t & 7);
				break;
				case 0x17:
					RES(0x80, t & 7);
				break;
				case 0x18:
					SET(0x01, t & 7);
				break;
				case 0x19:
					SET(0x02, t & 7);
				break;
				case 0x1A:
					SET(0x04, t & 7);
				break;
				case 0x1B:
					SET(0x08, t & 7);
				break;
				case 0x1C:
					SET(0x10, t & 7);
				break;
				case 0x1D:
					SET(0x20, t & 7);
				break;
				case 0x1E:
					SET(0x40, t & 7);
				break;
				case 0x1F:
					SET(0x80, t & 7);
				break;
				default:
					goto unhandled;
				break;
			}

			c.PC += 2;
		break;
		case 0xCC:  /* CALL Z, imm16 */
			if(flag_Z == 1)
			{
				c.SP -= 2;
				mem_write_word(c.SP, c.PC+3);
				c.PC = mem_get_word(c.PC+1);
				c.cycles += 6;
			} else {
				c.PC += 3;
				c.cycles += 3;
			}
		break;
		case 0xCD:  /* call imm16 */
			c.SP -= 2;
			mem_write_word(c.SP, c.PC+3);
			c.PC = mem_get_word(c.PC+1);
			c.cycles += 6;
		break;
		case 0xCE:  /* ADC a, imm8 */
			t = c.A;
			c.A = c.A + mem_get_byte(c.PC+1) + flag_C;
			set_Z(!c.A);
			set_N(0);
			//set_H()
			set_C(c.A < t);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0xD0:  /* RET NC */
			if(flag_C == 0)
			{
				c.PC = mem_get_word(c.SP);
				c.SP += 2;
				c.cycles += 3;
			} else {
				c.PC += 1;
				c.cycles += 1;
			}
		break;
		case 0xD1:  /* POP DE */
			s = mem_get_word(c.SP);
			set_DE(s);
			c.SP += 2;
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xD5:  /* PUSH DE */
			c.SP -= 2;
			mem_write_word(c.SP, get_DE());
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xD6:  /* SUB A, imm8 */
			t = mem_get_byte(c.PC+1);
			set_C(c.A > (c.A - t));
			c.A -= t;
			set_N(1);
			//set_H()
			set_Z(!c.A);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0xD9:  /* RETI */
			c.PC = mem_get_word(c.SP);
			c.SP += 2;
			c.cycles += 4;
			interrupt_enable();
		break;
		case 0xDF:  /* RST 18 */
			c.SP -= 2;
			mem_write_word(c.SP, c.PC);
			c.PC = 0x0018;
			c.cycles += 3;
		break;
		case 0xE0:  /* ld (FF00 + imm8), a */
			t = mem_get_byte(c.PC+1);
			mem_write_byte(0xFF00 + t, c.A);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0xE1:  /* POP HL */
			s = mem_get_word(c.SP);
			set_HL(s);
			c.SP += 2;
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xE2:  /* LD (FF00 + C), A */
			s = 0xFF00 + c.C;
			mem_write_byte(s, c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xE5:  /* PUSH HL */
			c.SP -= 2;
			mem_write_word(c.SP, get_HL());
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xE6:  /* and a, imm8 */
			t = mem_get_byte(c.PC+1);
			set_N(0);
			set_H(1);
			set_C(0);
			c.A = t & c.A;
			set_Z(!c.A);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0xE9:  /* JP HL */
			c.PC = get_HL();
			c.cycles += 1;
		break;
		case 0xEA:  /* LD (mem16), a */
			s = mem_get_word(c.PC+1);
			mem_write_byte(s, c.A);
			c.PC += 3;
			c.cycles += 4;
		break;
		case 0xEE:  /* XOR A, imm8 */
			c.A ^= mem_get_byte(c.PC+1);
			c.F = (!c.A)<<7;
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0xF0:  /* ld a, (FF00 + imm8) */
			t = mem_get_byte(c.PC+1);
			c.A = mem_get_byte(0xFF00 + t);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0xF1:  /* POP AF */
			s = mem_get_word(c.SP);
			set_AF(s);
			c.SP += 2;
			c.PC += 1;
			c.cycles += 3;
		break;
		break;
		case 0xF3:  /* DI */
			c.PC += 1;
			c.cycles += 1;
			interrupt_disable();
		break;
		case 0xF5:  /* PUSH AF */
			c.SP -= 2;
			mem_write_word(c.SP, get_AF());
			c.PC += 1;
			c.cycles += 3;
		break;
		case 0xF6:  /* OR A, imm8 */
			c.A |= mem_get_byte(c.PC+1);
			c.F = (!c.A)<<7;
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0xF8:  /* LD HL, SP + imm8 */
			t = mem_get_byte(c.PC+1);
			set_HL(c.SP + t);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0xFA:  /* LD A, (mem16) */
			s = mem_get_word(c.PC+1);
			c.A = mem_get_byte(s);
			c.PC += 3;
			c.cycles += 4;
		break;
		case 0xFB:  /* EI */
			c.PC += 1;
			c.cycles += 1;
			interrupt_enable();
		break;
		case 0xFE:  /* cp a, imm8 */
			t = mem_get_byte(c.PC+1);
			set_Z(c.A == t);
			set_N(1);
			set_C(c.A < t);
			/* FIXME: set_H */
			c.PC += 2;
			c.cycles += 2;
		break;
		default:
			unhandled:
			printf("Unhandled opcode %02X at %04X\n", b, c.PC);
			printf("cycles: %d\n", c.cycles);
			return 0;
		break;
	}

	return 1;
}
