#include <stdio.h>
#include <conio.h>
#include "256.h"
#include "bmp.h"

#define __WIDHT 450
#define __HEIGHT 440


int initgame(void) //用于设置显示模式为320*200 256色
{
 union REGS r;
 r.h.ah=0;
 r.h.al=0x101;
 int86(0x10,&r,&r);
 return 1;

}

void MyDelay(aaaax)
{

	     asm	push es
	     asm	push cx
	     asm	push ax

	     asm	sti
	     asm        mov cx,aaaax
	     asm	push 0x40
	     asm	pop  es

	loop0:
	     asm        mov  ax,word ptr es:[0x6C]

	loop1:
	     asm        cmp  ax,word ptr es:[0x6C]
	     asm	jz   loop1

	     asm	loop loop0


	     asm	pop ax
	     asm	pop cx
	     asm	pop es
	     asm        cli

	return ;

}


void waitmom()
{
	struct  time t;
	int i;
	gettime(&t);
	i = t.ti_sec;
	while(1)
	{
		if(i >= 58 )
		{
			gettime(&t);
			if(t.ti_sec >= 0 && t.ti_sec <=20)
			{
			 break;
			}
		}
		else
		{
			gettime(&t);
			if(t.ti_sec - i >= 2)
			{
				break;
			}
		}
	}
}


void main(int argv,char *argc[])
{
	if(argv>1)
	{
      //	initgame();
		set_mode(640,480,8);
		showbmps(0,0,argc[1]);

		fcolor=15;
		fillbox(__WIDHT-1,__HEIGHT-1,__WIDHT+142+2+1,__HEIGHT+15+1);
		fcolor=14;
		fillbox(__WIDHT,__HEIGHT,__WIDHT+142+2,__HEIGHT+15);
		fcolor=2;
		waitmom();
		for(int i=0;i<143;i+=5)
		{
			fillbox(__WIDHT+i,__HEIGHT,__WIDHT+i+2,__HEIGHT+15);
			MyDelay(1);
		}
		asm{mov ax,0x03;int 0x10};
	}
}