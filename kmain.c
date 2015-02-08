#include "frame_buffer.h"
#include "serial.h"
int kmain(void)
{
        unsigned char sentence[] = "Hello World";
	unsigned short sentence_length = 11;
	
//	fb_clear_screen();
//	fb_write(sentence,sentence_length);
        serial_write(sentence, sentence_length);
	return 0;
}


