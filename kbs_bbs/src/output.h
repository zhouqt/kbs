#ifndef __OUTPUT_H__
#define __OUTPUT_H__

struct __buffered_output;

typedef int (*output_handler_t)(char *buf, size_t buflen, void *arg);
typedef void (*flush_handler_t)(struct __buffered_output *output);

typedef struct __buffered_output
{
	char *buf;
	size_t buflen;
	char *outp;
	output_handler_t output;
	flush_handler_t flush;
} buffered_output_t;

#endif /* __OUTPUT_H__ */
