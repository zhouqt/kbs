
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

#define STATE_ESC_SET 0x01
#define STATE_FONT_SET    0x02
#define STATE_NEW_LINE 0x04
#define STATE_QUOTE_LINE 0x08
#define STATE_NONE   0x00

#define STATE_SET(s, b) (s |= b)
#define STATE_CLR(s, b) (s &= ~b)
#define STATE_ISSET(s, b) (s & b)
#define STATE_ZERO(s) (s = 0)

#define STYLE_SET_FG(s, c) (s = (s & ~0x07) | (c & 0x07))
#define STYLE_SET_BG(s, c) (s = (s & ~0x70) | ((c & 0x07) << 4))
#define STYLE_GET_FG(s) (s & 0x0F)
#define STYLE_GET_BG(s) ((s & 0x70) >> 4)
#define STYLE_CLR_FG(s) (s &= ~0x0F)
#define STYLE_CLR_BG(s) (s &= ~0xF0)
#define STYLE_ZERO(s) (s = 0)
#define STYLE_SET(s, b) (s |= b)
#define STYLE_CLR(s, b) (s &= ~b)
#define STYLE_ISSET(s, b) (s & b)

#define FONT_STYLE_UL     0x0100
#define FONT_STYLE_BLINK  0x0200
#define FONT_STYLE_ITALIC 0x0400

#define FONT_FG_BOLD      0x08
#define FONT_COLOR_BLACK     0x00
#define FONT_COLOR_RED       0x01
#define FONT_COLOR_GREEN     0x02
#define FONT_COLOR_YELLOW    0x03
#define FONT_COLOR_BULE      0x04
#define FONT_COLOR_MAGENTA   0x05
#define FONT_COLOR_CYAN      0x06
#define FONT_COLOR_WHITE     0x07

#define FONT_STYLE_QUOTE FONT_STYLE_ITALIC
#define FONT_COLOR_QUOTE FONT_COLOR_CYAN

#define FONT_BG_SET       0x80

#define STRLEN 80

typedef int (*output_handler_t)(char *buf, size_t buflen, void *arg);

typedef struct
{
	char *buf;
	size_t buflen;
	char *outp;
	output_handler_t output;
} buffered_output_t;

static char output_buf[4096];
static char *outp = output_buf;

static void flush_buffer(buffered_output_t *output)
{
	*(output->outp) = '\0'; 
	printf("%s", output->buf);
	output->outp = output->buf;
}

static int buffered_output(char *buf, size_t buflen, void *arg)
{
	buffered_output_t *output = (buffered_output_t *)arg;
	if (output->buflen < buflen)
	{
		printf("%s", buf);
		return 0;
	}
	if ((output->buflen - (output->outp - output->buf)) < buflen) 
		flush_buffer(output);
	strncpy(output->outp, buf, buflen); 
	output->outp += buflen;

	return 0;
}

static void print_font_style(unsigned int style, buffered_output_t *output)
{
	char font_class[8];
	char font_style[STRLEN];
	char font_str[256];
	unsigned int bg;

	if (STYLE_ISSET(style, FONT_BG_SET))
	{
		bg = 8;
	}
	else
		bg = STYLE_GET_BG(style);
	sprintf(font_class, "f%01d%02d", bg, STYLE_GET_FG(style));
	font_style[0] = '\0';
	if (STYLE_ISSET(style, FONT_STYLE_UL))
		strcat(font_style, "text-decoration: underline; ");
	if (STYLE_ISSET(style, FONT_STYLE_ITALIC))
		strcat(font_style, "font-style: italic; ");
	if (font_style[0] != '\0')
		sprintf(font_str, "<font class=\"%s\" style=\"%s\">", 
				font_class, font_style);
	else
		sprintf(font_str, "<font class=\"%s\">", font_class);
	output->output(font_str, strlen(font_str), output);
}

static void html_output(char *buf, size_t buflen, buffered_output_t *output)
{
	size_t i;
	
	for (i = 0; i < buflen; i++)
	{
		switch (buf[i])
		{
		case '&':
			output->output("&amp;", 5, output);
			break;
		case '<':
			output->output("&lt;", 4, output);
			break;
		case '>':
			output->output("&gt;", 4, output);
			break;
		case ' ':
			output->output("&nbsp;", 6, output);
			break;
		default:
			output->output(&buf[i], 1, output);
		}
	}
}

static void print_raw_ansi(char *buf, size_t buflen, buffered_output_t *output)
{
	size_t i;
	
	for (i = 0; i < buflen; i++)
	{
		if (buf[i] == 0x1b)
			html_output("*", 1, output);
		else
			html_output(&buf[i], 1, output);
	}
}

static void generate_font_style(unsigned int *style, unsigned int *ansi_val,
								size_t len)
{
	size_t i;
	unsigned int color;

	for (i = 0; i < len; i++)
	{
		if (ansi_val[i] == 0)
			STYLE_ZERO(*style);
		else if (ansi_val[i] == 1)
			STYLE_SET(*style, FONT_FG_BOLD);
		else if (ansi_val[i] == 4)
			STYLE_SET(*style, FONT_STYLE_UL);
		else if (ansi_val[i] == 5)
			STYLE_SET(*style, FONT_STYLE_BLINK);
		else if (ansi_val[i] >= 30 && ansi_val[i] <= 37)
		{
			if (ansi_val[i] == 30)
				color = 7;
			else if (ansi_val[i] == 37)
				color = 0;
			else
				color = ansi_val[i] - 30;
			STYLE_SET_FG(*style, color);
		}
		else if (ansi_val[i] >= 40 && ansi_val[i] <= 47)
		{
			/* user explicitly specify background color */
			/* STYLE_SET(*style, FONT_BG_SET); */
			color = ansi_val[i] - 40;
			STYLE_SET_BG(*style, color);
		}
	}
}

void print_ansi(char *buf, size_t buflen, buffered_output_t *output)
{
	unsigned int font_style = 0;
	unsigned int ansi_state;
	unsigned int ansi_val[STRLEN];
	int ival = 0;
	size_t i;
	char *ptr = buf;
	char *ansi_begin;
	char *ansi_end;
	
	if (ptr == NULL)
		return;
	STATE_ZERO(ansi_state);
	bzero(ansi_val, sizeof(ansi_val));
	for (i = 0; i < buflen; i++)
	{
		if (STATE_ISSET(ansi_state, STATE_NEW_LINE))
		{
			STATE_CLR(ansi_state, STATE_NEW_LINE);
			if (i < (buflen - 1) && (buf[i] == ':' && buf[i+1] == ' '))
			{
				STATE_SET(ansi_state, STATE_QUOTE_LINE);
				if (STATE_ISSET(ansi_state, STATE_FONT_SET))
					output->output("</font>", 7, output);
				/* set quoted line styles */
				STYLE_SET(font_style, FONT_STYLE_QUOTE);
				STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
				STYLE_CLR_BG(font_style);
				print_font_style(font_style, output);
				output->output(&buf[i], 1, output);
				STATE_SET(ansi_state, STATE_FONT_SET);
				STATE_CLR(ansi_state, STATE_ESC_SET);
				/* clear ansi_val[] array */
				bzero(ansi_val, sizeof(ansi_val));
				ival = 0;
				continue;
			}
			else
				STATE_CLR(ansi_state, STATE_QUOTE_LINE);
		}
		if (i < (buflen - 1) && (buf[i] == 0x1b && buf[i+1] == '['))
		{
			if (STATE_ISSET(ansi_state, STATE_ESC_SET))
			{
				/* *[*[ or *[13;24*[ */
				size_t len;
				ansi_end = &buf[i - 1];
				len = ansi_end - ansi_begin + 1;
				print_raw_ansi(ansi_begin, len, output);
			}
			STATE_SET(ansi_state, STATE_ESC_SET);
			ansi_begin = &buf[i];
			i++; /* skip the next '[' character */
		}
		else if (buf[i] == '\n')
		{
			if (STATE_ISSET(ansi_state, STATE_ESC_SET))
			{
				/* *[\n or *[13;24\n */
				size_t len;
				ansi_end = &buf[i - 1];
				len = ansi_end - ansi_begin + 1;
				print_raw_ansi(ansi_begin, len, output);
				STATE_CLR(ansi_state, STATE_ESC_SET);
			}
			if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE))
			{
				/* end of a quoted line */
				output->output("</font>", 7, output);
				STYLE_CLR(font_style, FONT_STYLE_QUOTE);
				STATE_CLR(ansi_state, STATE_FONT_SET);
			}
			output->output("<br />\n", 7, output);
			STATE_CLR(ansi_state, STATE_QUOTE_LINE);
			STATE_SET(ansi_state, STATE_NEW_LINE);
		}
		else
		{
			if (STATE_ISSET(ansi_state, STATE_ESC_SET))
			{
				if (buf[i] == 'm')
				{
					/* *[0;1;4;31m */
					if (STATE_ISSET(ansi_state, STATE_FONT_SET))
					{
						output->output("</font>", 7, output);
						STATE_CLR(ansi_state, STATE_FONT_SET);
					}
					if (i < buflen - 1)
					{
						generate_font_style(&font_style, ansi_val, ival+1);
						if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE))
							STYLE_SET(font_style, FONT_STYLE_QUOTE);
						print_font_style(font_style, output);
						STATE_SET(ansi_state, STATE_FONT_SET);
						STATE_CLR(ansi_state, STATE_ESC_SET);
						/*STYLE_ZERO(font_style);*/
						/* clear ansi_val[] array */
						bzero(ansi_val, sizeof(ansi_val));
						ival = 0;
					}
				}
				else if (isalpha(buf[i]))
				{
					/* *[23;32H */
					/* ignore it */
					STATE_CLR(ansi_state, STATE_ESC_SET);
					STYLE_ZERO(font_style);
					/* clear ansi_val[] array */
					bzero(ansi_val, sizeof(ansi_val));
					ival = 0;
					continue;
				}
				else if (buf[i] == ';')
				{
					if (ival < sizeof(ansi_val) - 1)
					{
						ival ++; /* go to next ansi_val[] element */
						ansi_val[ival] = 0;
					}
				}
				else if (buf[i] >= '0' && buf[i] <= '9')
				{
					ansi_val[ival] *= 10;
					ansi_val[ival] += (buf[i] - '0');
				}
				else
				{
					/* *[1;32/XXXX or *[* or *[[ */
					/* not a valid ANSI string, just output it */
					size_t len;
					
					ansi_end = &buf[i];
					len = ansi_end - ansi_begin + 1;
					print_raw_ansi(ansi_begin, len, output);
					STATE_CLR(ansi_state, STATE_ESC_SET);
					/* clear ansi_val[] array */
					bzero(ansi_val, sizeof(ansi_val));
					ival = 0;
				}
				
			}
			else
				print_raw_ansi(&buf[i], 1, output);
		}
	}
	if (STATE_ISSET(ansi_state, STATE_FONT_SET))
	{
		output->output("</font>", 7, output);
		STATE_CLR(ansi_state, STATE_FONT_SET);
	}
	flush_buffer(output);
}

int main()
{
	char *str;
	int fd;
	struct stat st;
	buffered_output_t out;

	if ((out.buf = (char *)malloc(4096)) == NULL)
		return -1;
	out.outp = out.buf;
	out.buflen = 4096;
	out.output = buffered_output;
	if ((fd = open("ansi.txt", O_RDONLY, 0644)) < 0)
		return -1;
	fstat(fd, &st);
	str = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	print_ansi(str, st.st_size, &out);
	munmap(str, st.st_size);
	free(out.buf);
	printf("\n");
	return 0;
}

