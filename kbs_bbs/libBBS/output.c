#include "bbs.h"
#include "output.h"

static int default_write_func(const char *str, unsigned int len)
{
	fwrite(str, 1, len, stdout);
	return len;
}

buffered_output_t* alloc_output(unsigned int buflen)
{
	buffered_output_t *ptr;

	if ((ptr = (buffered_output_t *)malloc(sizeof(buffered_output_t))) == NULL)
		return NULL;
	if ((ptr->buf = (char *)malloc(buflen+1)) == NULL)
	{
		free(ptr);
		return NULL;
	}
	ptr->outp = ptr->buf;
	ptr->buflen = buflen;
	/*ptr->output = default_buffered_output;
	ptr->flush = default_flush_buffer;*/
	ptr->write = default_write_func;

	return ptr;
}

void free_output(buffered_output_t *out)
{
	if (out != NULL)
	{
		free(out->buf);
		free(out);
	}
}

output_write_func_t 
override_default_write(buffered_output_t *out, output_write_func_t write_func)
{
	output_write_func_t old_write = out->write;
	out->write = write_func;

	return old_write;
}

