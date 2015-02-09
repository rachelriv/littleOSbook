#include "frame_buffer.h"
#include "serial.h"
int kmain(void)
{
        unsigned char sentence[] = "yo yo yo what up";
	unsigned short sentence_length = 16;
	
//	fb_clear_screen();
//	fb_write(sentence,sentence_length);
        serial_write(sentence, sentence_length);
	return 0;
}


