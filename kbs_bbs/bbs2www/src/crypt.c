#include "internal/crypt.h"

static int des_set_key(des_cblock *key, struct des_ks_struct *schedule) {
	register unsigned long c,d,t,s;
	register unsigned char *in;
	register unsigned long *k;
	register int i;
	k=(unsigned long *)schedule;
	in=(unsigned char *)key;
	c2l(in,c);
	c2l(in,d);
	PERM_OP (d,c,t,4,0x0f0f0f0f);
	HPERM_OP(c,t,-2,0xcccc0000);
	HPERM_OP(d,t,-2,0xcccc0000);
	PERM_OP (d,c,t,1,0x55555555);
	PERM_OP (c,d,t,8,0x00ff00ff);
	PERM_OP (d,c,t,1,0x55555555);
	d=(((d&0x000000ff)<<16)| (d&0x0000ff00) | ((d&0x00ff0000)>>16)|((c&0xf0000000)>>4));
	c&=0x0fffffff;
	for(i=0; i<ITERATIONS; i++) {
		if (shifts2[i]) {
			c=((c>>2)|(c<<26)); 
			d=((d>>2)|(d<<26)); 
		} else {
			c=((c>>1)|(c<<27)); 
			d=((d>>1)|(d<<27)); 
		}
		c&=0x0fffffff;
		d&=0x0fffffff;
		s=	skb[0][ (c    )&0x3f                ]|
			skb[1][((c>> 6)&0x03)|((c>> 7)&0x3c)]|
			skb[2][((c>>13)&0x0f)|((c>>14)&0x30)]|
			skb[3][((c>>20)&0x01)|((c>>21)&0x06) |
			                      ((c>>22)&0x38)];
		t=	skb[4][ (d    )&0x3f                ]|
			skb[5][((d>> 7)&0x03)|((d>> 8)&0x3c)]|
			skb[6][ (d>>15)&0x3f                ]|
			skb[7][((d>>21)&0x0f)|((d>>22)&0x30)];

		*(k++)=((t<<16)|(s&0x0000ffff))&0xffffffff;
		s=     ((s>>16)|(t&0xffff0000));
		
		s=(s<<4)|(s>>28);
		*(k++)=s&0xffffffff;
	}
	return 0;
}


static int body(unsigned long *out0, unsigned long *out1, des_key_schedule ks, unsigned long Eswap0, unsigned long Eswap1) {
        register unsigned long l, r, t, u;
        register unsigned long *s;
        register int i, j;
        register unsigned long E0, E1;
        l=0;
        r=0;
        s=(unsigned long *)ks;
        E0=Eswap0;
        E1=Eswap1;
        for(j=0; j<25; j++) {
                for(i=0; i<(ITERATIONS*2); i+=4) {
                        D_ENCRYPT(l, r, i);
                        D_ENCRYPT(r, l, i+2);
                }
                t=l;
                l=r;
                r=t;
        }
        t=r;
        r=(l>>1)|(l<<31);
        l=(t>>1)|(t<<31);
        l&=0xffffffff;
        r&=0xffffffff;
        PERM_OP(r, l, t, 1, 0x55555555);
        PERM_OP(l, r, t, 8, 0x00ff00ff);
        PERM_OP(r, l, t, 2, 0x33333333);
        PERM_OP(l, r, t,16, 0x0000ffff);
        PERM_OP(r, l, t, 4, 0x0f0f0f0f);
        *out0=l;
        *out1=r;
        return 0;
}

char *crypt1(char *buf, char *salt) {
	unsigned int i, j, x, y;
	unsigned long Eswap0=0, Eswap1=0;
	unsigned long out[2], ll;
	des_cblock key;
	des_key_schedule ks;
	static unsigned char buff[20];
	unsigned char bb[9];
	unsigned char *b=bb;
	unsigned char c,u;
	x=buff[0]=((salt[0] == '\0')?'A':salt[0]);
	Eswap0=con_salt[x];
	x=buff[1]=((salt[1] == '\0')?'A':salt[1]);
	Eswap1=con_salt[x]<<4;
	for(i=0; i<8; i++) {
		c= *(buf++);
		if (!c) break;
		key[i]=(c<<1);
	}
	for(; i<8; i++)
		key[i]=0;
	des_set_key((des_cblock *)(key), ks);
	body(&(out[0]), &(out[1]), ks, Eswap0, Eswap1);
	ll=out[0]; 
	l2c(ll, b);
	ll=out[1]; 
	l2c(ll, b);
	y=0;
	u=0x80;
	bb[8]=0;
	for(i=2; i<13; i++) {
		c=0;
		for(j=0; j<6; j++) {
			c<<=1;
			if(bb[y] & u) c|=1;
			u>>=1;
			if(!u) {
				y++;
				u=0x80;
			}
		}
		buff[i]=cov_2char[c];
	}
	buff[13]=0;
	return (char*) buff;
}
