#ifndef __OUTPUT_H__
#define __OUTPUT_H__

struct __buffered_output;

typedef int (*output_handler_t)(char *buf, size_t buflen, void *arg);
typedef void (*flush_handler_t)(struct __buffered_output *output);
typedef int (*output_write_func_t)(const char *str, size_t len);

typedef struct __buffered_output
{
	char *buf;
	size_t buflen;
	char *outp;
	/*output_handler_t output;
	flush_handler_t flush;*/
	output_write_func_t write;
} buffered_output_t;

#define BUFFERED_FLUSH(o) \
{ \
	if ((o)->outp - (o)->buf > 0) \
	{ \
		(o)->write((o)->buf, (o)->outp - (o)->buf); \
		(o)->outp = (o)->buf; \
	} \
}

#define BUFFERED_OUTPUT(o,p,l) \
{ \
	if ((o)->buflen < l) \
	{ \
		BUFFERED_FLUSH(o); \
		(o)->write(p, l); \
	} \
	else \
	{ \
		if ((o)->buflen - ((o)->outp - (o)->buf) < l) \
			BUFFERED_FLUSH(o); \
		memcpy((o)->outp, p, l); \
		(o)->outp += l; \
	} \
}

#endif /* __OUTPUT_H__ */
