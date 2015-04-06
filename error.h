#include <stdint.h>
#define ERROR(msg) error(msg, __FILE__, __LINE__);
extern void error(const char *message, const char *file, uint32_t line);

