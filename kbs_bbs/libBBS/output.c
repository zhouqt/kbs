#include "bbs.h"
#include "output.h"

static void default_flush_buffer(buffered_output_t *output)
{
	*(output->outp) = '\0'; 
	printf("%s", output->buf);
	output->outp = output->buf;
}

static int default_buffered_output(char *buf, size_t buflen, void *arg)
{
	buffered_output_t *output = (buffered_output_t *)arg;
	if (output->buflen <= buflen)
	{
		output->flush(output);
		printf("%s", buf);
		return 0;
	}
	if ((output->buflen - (output->outp - output->buf) - 1) <= buflen) 
		output->flush(output);
	strncpy(output->outp, buf, buflen); 
	output->outp += buflen;

	return 0;
}

buffered_output_t* alloc_output(size_t buflen)
{
	buffered_output_t *ptr;

	if ((ptr = (buffered_output_t *)malloc(sizeof(buffered_output_t))) == NULL)
		return NULL;
	if ((ptr->buf = (char *)malloc(buflen)) == NULL)
	{
		free(ptr);
		return NULL;
	}
	ptr->outp = ptr->buf;
	ptr->buflen = buflen;
	ptr->output = default_buffered_output;
	ptr->flush = default_flush_buffer;

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

output_handler_t 
override_default_output(buffered_output_t *out, output_handler_t output)
{
	output_handler_t old_output = out->output;
	out->output = output;

	return old_output;
}

flush_handler_t 
override_default_flush(buffered_output_t *out, flush_handler_t flush)
{
	flush_handler_t old_flush = out->flush;
	out->flush = flush;

	return old_flush;
}

