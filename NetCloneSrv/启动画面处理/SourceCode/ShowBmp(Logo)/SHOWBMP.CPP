#include "stdio.h"
#include "conio.h"
#include "256.h"
#include "bmp.h"

void main(int argv,char *argc[])
{
	if(argv>1){
		printf("press any key to begin......\n");getch();
		set_mode(640,480,8);
		showbmps(0,0,argc[1]);
		fcolor=1;
		getch();
		for(int i=0;i<143;i++){
			//fillbox(281+i,425,300+i+26,437);
			fillbox(281+i,425,281+i+2,437);
			delay(10);
		}
		getch();
		asm{mov ax,0x03;int 0x10};
	}else{
		printf("Required parameter missing...\n");
	}
}