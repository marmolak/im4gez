#include <dos.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>

#include <cstdio>
#include <cstdlib>

/* VGA Palette access port addresses */
#define PALETTE_MASK  0x3C6
#define PALETTE_READ  0x3C7
#define PALETTE_WRITE 0x3C8
#define PALETTE_DATA  0x3C9

void switch_mode(char mode)
{
    union REGS i_regs;
    union REGS o_regs;
    struct SREGS s_regs;
    i_regs.h.ah = 0x00;
    i_regs.h.al = mode;
    //i_regs.x.ax = mode;
    int86x(0x10, &i_regs, &o_regs, &s_regs);
}

void switch_graphics()
{
	// 0x13 -> 320x200 256
	switch_mode(0x13);
}


void switch_text()
{
	switch_mode(0x03);
}

struct RGB_Color {
      unsigned char red;
      unsigned char green;
      unsigned char blue;
};

RGB_Color old_pallete[256];
void save_pallete()
{
	_disable();

	outp(PALETTE_MASK, 0xFF);

	for (unsigned short i = 0; i < 256; ++i)
	{
	      /* Send color palette register index to write to */
	      outp(PALETTE_READ, i);

	      /* Write the color palette entry */
	      old_pallete[i].red 	= inp(PALETTE_DATA); 
	      old_pallete[i].green 	= inp(PALETTE_DATA);
	      old_pallete[i].blue 	= inp(PALETTE_DATA); 
	 }

	_enable();
}

void fill_2()
{
	_disable();

	outp(PALETTE_MASK, 0xFF);

	for (unsigned short i = 0; i < 256; ++i)
	{
	      /* Send color palette register index to write to */
	      outp(PALETTE_WRITE, i);

	      /* Write the color palette entry */
	      outp(PALETTE_DATA, old_pallete[i].red); 
	      outp(PALETTE_DATA, old_pallete[i].green);
	      outp(PALETTE_DATA, old_pallete[i].blue); 
	 }

	_enable();

}

void fill_pallete()
{
	_disable();

	outp(PALETTE_MASK, 0xFF);

	for (unsigned short i = 0; i < 256; ++i)
	{
	      /* Send color palette register index to write to */
	      outp(PALETTE_WRITE, i);

	      /* Write the color palette entry */
	      outp(PALETTE_DATA, 0x00); 
	      outp(PALETTE_DATA, 0x00);
	      outp(PALETTE_DATA, 0x00); 
	 }

	_enable();
}


int main()
{
	int handle = -1;
	if(_dos_open("image.raw", O_RDONLY, &handle ) != 0)
	{
		return 1;
	}

	switch_graphics();
	save_pallete();
	fill_pallete();

	unsigned char far *p = (unsigned char far *)0xA0000000L;
	unsigned char far *p_end = (unsigned char far *) (0xA0000000L + (320 * 200));

	do
	{
		unsigned len_read;
		_dos_read(handle, p, 320, &len_read);
		p += 320;
	} while (p < p_end);
	
	_dos_close(handle);

	fill_2();

	std::getchar();
	switch_text();
    	return EXIT_SUCCESS;
}

