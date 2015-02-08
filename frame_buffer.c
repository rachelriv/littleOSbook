#include "frame_buffer.h"
#include "io.h"
static struct framebuffer *fb = (struct framebuffer *) 0x000B8000;
static unsigned short cursor_idx = 0;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
	
    fb[i].c = c;
    fb[i].colors = ((fg & 0x0F) << 4) | (bg & 0x0F);
}
void fb_move_cursor(unsigned short pos)
{
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_CMD);
	outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_CMD);
	outb(FB_DATA_PORT, pos & 0x00FF);
}
void fb_write_sentence(
	char * sentence,
	unsigned short sentence_len, 
	unsigned char fg, 
	unsigned char bg
) {
	int i;
	//int start_point = start_row*SCREEN_WIDTH + start_col;
	//i = 0;
	//i = sentence_len - sentence_len;
	for (i=0; i<sentence_len;i++)
	{
		fb_write_cell(cursor_idx,sentence[i],bg,fg);
                cursor_idx = (cursor_idx + 2) % FB_NUM_CELLS;
	}
        fb_move_cursor(cursor_idx);
}

void fb_clear_screen(void){
	int i,j;
	for(i = 0; i < SCREEN_WIDTH; i++)
	{
		for(j = 0; j < SCREEN_HEIGHT; j++)
		{
			fb_write_cell( (j*SCREEN_WIDTH + i)*2,' ',FB_COLOR_BLACK,FB_COLOR_BLACK);
		}
	}
}

void fb_write(unsigned char *buf, unsigned short len){
	unsigned int i;
	unsigned char fg = FB_COLOR_CYAN;
	unsigned char bg = FB_COLOR_GREEN;
	for (i = 0; i < len; ++i)
	{
		fb_write_cell(cursor_idx, buf[i], fg, bg);
		cursor_idx = (cursor_idx + 1) % FB_NUM_CELLS;
	}
	fb_move_cursor(cursor_idx);
}
