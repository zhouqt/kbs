
static const char* to_b64 =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int b64dec(const char* string, char* buff)
{
	int size = 0;

	if (string)
	{
		int length = strlen(string);

		/* do a format verification first */
		if (length > 0)
		{
			int count = 0, rem = 0;
			const char* tmp = string;

			while (length > 0)
			{
				int skip = strspn(tmp, to_b64);
				count += skip;
				length -= skip;
				tmp += skip;
				if (length > 0)
				{
					int i, vrfy = strcspn(tmp, to_b64);

					for (i = 0; i < vrfy; i++)
					{
						if (isspace(tmp[i]))
						continue;

						if (tmp[i] == '=')
						{
							/* we should check if we're close to the end of the string */
							rem = count % 4;

							/* rem must be either 2 or 3, otherwise no '=' should be here */
							if (rem < 2)
							return 0;

							/* end-of-message recognized */
							break;
						}
						else
						{
							/* Transmission error; RFC tells us to ignore this, but:
							*  - the rest of the message is going to even more corrupt since we're sliding bits out of place
							* If a message is corrupt, it should be dropped. Period.
							*/

							return 0;
						}
					}

					length -= vrfy;
					tmp += vrfy;
				}
			}

			size = (count / 4) * 3 + (rem ? (rem - 1) : 0);

			if (buff)
			{
				if (count > 0)
				{
					int i, qw = 0, tw = 0;
					byte* data = (byte*)buff;

					length = strlen(tmp = string);

					for (i = 0; i < length; i++)
					{
						register char ch = string[i];
						register byte bits;

						if (isspace(ch))
						continue;

						bits = 0;
						if ((ch >= 'A') && (ch <= 'Z'))
						{
							bits = (byte) (ch - 'A');
						}
						else if ((ch >= 'a') && (ch <= 'z'))
						{
							bits = (byte) (ch - 'a' + 26);
						}
						else if ((ch >= '0') && (ch <= '9'))
						{
							bits = (byte) (ch - '0' + 52);
						}
						else if (ch=='+') {
							bits = 62;	
						}
						else if (ch=='/') {
							bits = 63;
						}
						else if (ch == '=')
						break;

						switch (qw++)
						{
							case 0:
							data[tw+0] = (bits << 2) & 0xfc;
							break;
							case 1:
							data[tw+0] |= (bits >> 4) & 0x03;
							data[tw+1] = (bits << 4) & 0xf0;
							break;
							case 2:
							data[tw+1] |= (bits >> 2) & 0x0f;
							data[tw+2] = (bits << 6) & 0xc0;
							break;
							case 3:
							data[tw+2] |= bits & 0x3f;
							break;
						}

						if (qw == 4)
						{
							qw = 0;
							tw += 3;
						}
					}
				}
			}
		}
	}

	return size;
}

#define SKIP '\202'
#define NOSKIP 'A'

const char hexmap[] = {
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
      0 ,    1 ,    2 ,    3 ,    4 ,    5 ,    6 ,    7 ,
      8 ,    9 ,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,    10,    11,    12,    13,    14,    15,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,
     SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP,   SKIP
};


int qpdec(const char* string, char* buff)
{
	const char *s;
	char *result = buff;
	int i;
	char mid[3];
	bool ok = true;
	s = string;
	
	while (*s != '\0') //loop through the entire string...
	{
		if (*s == '=') //woops, needs to be decoded...
		{
			for (i = 0; i < 3; i++) //is s more than 3 chars long...
			{
				if (s[i] == '\0')
				{
					return 0;
				}
			}
			s++; //move past the "="
			//let's put the hex part into mid...
			for (i = 0; i < 2; i++)
			{
				if (hexmap[(int)s[i]] == SKIP)
				{
					//we have an error, or a linebreak, in the encoding...
					ok = false;
					if (s[i] == '\r' && s[i + 1] == '\n')
					{
						s += 2;
						//*(result++) = '\r';
						//*(result++) = '\n';
						break;
					}
					else
					{
						//we have an error in the encoding...
						//s--;
					}
				}
				mid[i] = s[i];
			}
			//now we just have to convert the hex string to an char...
			if (ok)
			{
				int m;
				s += 2;
				m = hexmap[(int)mid[0]];
				m <<= 4;
				m |= hexmap[(int)mid[1]];
				*(result++) = m;
			}
		}
		else
		{
			if (*s != '\0') *(result++) = *(s++);
		}
	}
	*(result++)=0;
	return strlen(string);
}

