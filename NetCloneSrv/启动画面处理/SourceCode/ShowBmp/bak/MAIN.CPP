//***********************************************************************
#include<stdio.h>
#include<conio.h>
#include<iostream.h>
#include<dos.h>
#include<alloc.h>

//包含上面的DRAW.CPP，我在编译的时候，将DRAW.CPP放在D:\CODE\TC3\GAME1文件夹里面，你也可以放在别处
#include<D:\CODE\TC3\GAME1\DRAW.CPP>

int initgame(void) //用于设置显示模式为320*200 256色
{
	union REGS r;
	r.h.ah=0;
	r.h.al=0x13;
	int86(0x10,&r,&r);
	return 1;
}

void exitgame(void) //用于设置回80*25文本模式
{
	union REGS r;
	r.h.ah=0;
	r.h.al=0x03;
	int86(0x10,&r,&r);
	return;
}

void main(void)
{
	if(!initgame())
	{
		cout<<"initgame error"<<endl;
		cout<<"prese any key to exit...";
		getch();
		return;
	}
	FILE *fp;
	fp=fopen("PIC\\test.bmp","rb");
	if(fp==NULL)
	{
		cout<<"open file error";
		getch();
		return;
	}
	DrawImage(fp,20,20,0,0,32,48,253);
	getch();
	fclose(fp);
	exitgame();
	
}
//***********************************************************************
