#ifndef __OUTPUT_H__
#define __OUTPUT_H__

typedef int (*output_handler_t)(char *buf, size_t buflen, void *arg);

typedef struct
{
	char *buf;
	size_t buflen;
	char *outp;
	output_handler_t output;
} buffered_output_t;

#endif /* __OUTPUT_H__ */