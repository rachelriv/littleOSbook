#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT            0x3D5
#define FB_HIGH_BYTE_CMD    14
#define FB_LOW_BYTE_CMD     15

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define FB_NUM_CELLS (24*80)

#define FB_COLOR_BLACK 0
#define FB_COLOR_BLUE 1
#define FB_COLOR_GREEN 2
#define FB_COLOR_CYAN 3
#define FB_COLOR_RED 4
#define FB_COLOR_MAGENTA 5
#define FB_COLOR_BROWN 6
#define FB_COLOR_LIGHT_GREY 7
#define FB_COLOR_DARK_GREY 8
#define FB_COLOR_LIGHT_BLUE 9
#define FB_COLOR_LIGHT_GREEN 10
#define FB_COLOR_LIGHT_CYAN 11
#define FB_COLOR_LIGHT_RED 12
#define FB_COLOR_LIGHT_MAGENTA 13
#define FB_COLOR_LIGHT_BROWN 14
#define FB_COLOR_WHITE 15

struct framebuffer
{
	char		c;
	unsigned char	colors;
} __attribute__((packed));
/** fb_write_cell:
 *  Writes a character with the given foreground and background to position i
 *  in the framebuffer.
 *
 *  @param i  The location in the framebuffer
 *  @param c  The character
 *  @param fg The foreground color
 *  @param bg The background color
 */
void fb_write_cell(unsigned int i, char c, unsigned char bg, unsigned char fg);

/** fb_write_sentence:
 *  Writes a set of character with the given foreground and background to position i
 *  to the framebuffer.
 *  @param sentence 	The character buffer to output to the screen
 *  @param sentence_len The length of the character buffer
 *  @param start_row 	The start row for outputing
 *  @param start_col 	The start col for outputing
 *  @param fg The foreground color
 *  @param bg The background color
 */
void fb_move_cursor(unsigned short pos);
void fb_write_sentence(
	char * sentence,
	unsigned short sentence_len,
	unsigned char bg, 
	unsigned char fg
);


void fb_clear_screen(void);
void fb_write(unsigned char *buf, unsigned short len);
