#include "stdio.h"
#include "conio.h"
#include "256.h"
#include "bmp.h"


void main(int argv,char * argc[])
{
	struct bmphead *p;
	int bmpfile;
//	clrscr();
	if (argv<2){printf("Missing required bitmap filename.\nUsage:\n\t%s bitmap_filename\n",argc[0]);return;}
	if (argv==2){
		bmpfile=open(argc[1],O_RDONLY|O_BINARY);
		if(bmpfile!=-1)
		{
			read(bmpfile,p,54);
			close(bmpfile);
			if(p->signal==0x4d42)//[BM] flag
			{
//				printf("%d\n",p->bfsize);
				p->signal=0x5054;
				p->bfsize=p->bfsize^0x0731;
				p->bisize=p->bisize^0x130;
				p->biwidth=p->biwidth^0x0731;
				p->biheight=p->biheight^0x2259;
				p->planes=p->planes^0x1980;
				p->bitcount=p->bitcount^0x0127;
				p->compression=p->compression^0x1234;
//				printf("%d\n",p->bfsize);
//				printf("%d\n",p->bfsize^0x88481234);
//				printf("%d\n",p->bfsize^0x834);
//				getch();
				bmpfile=open(argc[1],O_RDWR|O_BINARY);
				if(bmpfile!=-1)
				{
					lseek(bmpfile,0L,SEEK_SET);
					write(bmpfile,p,54);
					close(bmpfile);
					printf("Conv success!\n");
				}else printf("Conv fail!\n");
			}
			else if(p->signal==0x5054)//[TP] flag
			{
				printf("Not need conv\n");
				return;
			}
		}
	}
}