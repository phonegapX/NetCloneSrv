#include <dos.h>
#include <FCNTL.H>
#include <io.h>
#include <string.h>
#include <math.h>
#include <alloc.h>
#include <conio.h>

//控制变量
unsigned Max_x=640,Max_y=480,Hi_Max_x=1280,vram=0;
unsigned char fcolor,bcolor,showmode=0,writemode=0x90;
unsigned hicolor,gmode=0x101;
int page;
int hzkems,ems=0;
int char_range;
char efont24[80]="ascii24.",tfont24[80]="hzk24t.",cfont24[80]="hzk24k.";
char efont32=0,tfont32[80]="asc32q.",cfont32[80]="xkj32";
unsigned char far *fe,far *fc;
struct type_1 {
  unsigned int offset;
  int pageno;
};
union type_2 {
  unsigned long addr;
  type_1 O_P;
} dp;

union type_3 {
  unsigned char far *ptr;
  type_1 O_P;
} dptr={(unsigned char far *)0xa0000000};
union type_4 {
  unsigned far *ptr;
  type_1 O_P;
} dhiptr={(unsigned far *)0xa0000000};

//测试显卡是否支持VESA BIOS, 不支持返回0, 否则返回值为显存容量(单位K),
unsigned testvesa()
{
  unsigned iseg,ioff;
  unsigned char info[256];
  iseg=FP_SEG(info);
  ioff=FP_OFF(info);
  asm {
    mov ax,0x4f00
    mov es,iseg
    mov di,ioff
    int 0x10
    mov vram,ax
  }
  if (vram==0x4f)
  {
    vram=info[19];
    vram<<=8;
    vram+=info[18];
  }
  else vram=0;
  return(vram);
}

//根据全程变量gmode设置显示模式
void clearscreen()
{
  asm {
    mov bx,gmode
    mov ax,0x4f02
    int 0x10
  }
  page=0;
}

//256色画点函数
void dpoint()
{
  if (page!=dp.O_P.pageno)
  {
    page=dp.O_P.pageno;
    asm {
      mov dx,page
      mov bx,0
      mov ax,0x4f05
      int 0x10
    }
  }
  dptr.O_P.offset=dp.O_P.offset;
  *dptr.ptr=fcolor;
}

//32K色画点函数
void hidpoint()
{           
  if (page!=dp.O_P.pageno)
  {
    page=dp.O_P.pageno;
    asm {
      mov dx,page
      mov bx,0
      mov ax,0x4f05
      int 0x10
    }
  }
  dhiptr.O_P.offset=dp.O_P.offset;
  *dhiptr.ptr=hicolor;
}

//256色画点函数
void point(int x,int y)
{
  type_2 p;
  unsigned char far *ptr;
  p.addr=y;
  p.addr=p.addr*Max_x+x;
  if (page!=p.O_P.pageno)
  {
    page=p.O_P.pageno;
    asm {
      mov dx,page
      mov bx,0
      mov ax,0x4f05
      int 0x10
    }
  }
  ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
  *ptr=fcolor;
}
void xpoint(int x,int y)
{
  type_2 p;
  unsigned char far *ptr;
  p.addr=y;
  p.addr=p.addr*Max_x+x;
  if (page!=p.O_P.pageno)
  {
    page=p.O_P.pageno;
    asm {
      mov dx,page
      mov bx,0
      mov ax,0x4f05
      int 0x10
    }
  }
  ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
  *ptr=~(*ptr);
}

//32K色画点函数
void hipoint(int x,int y)
{
  type_2 p;
  unsigned far *ptr;
  p.addr=y;
  p.addr=p.addr*Hi_Max_x+(x<<1);
  if (page!=p.O_P.pageno)
  {
    page=p.O_P.pageno;
    asm {
      mov dx,page
      mov bx,0
      mov ax,0x4f05
      int 0x10
    }
  }
  ptr=(unsigned far *)(0xa0000000+p.O_P.offset);
  *ptr=hicolor;
}

//256色画线函数
void line256(int x1,int y1,int x2,int y2)
{
  register int t,distance;
  int xerr=0,yerr=0,delta_x,delta_y;
  int incx,incy;
  delta_x=x2-x1;
  delta_y=y2-y1;
  if (delta_x>0) incx=1;
  else
    if (delta_x==0) incx=0;
    else
      incx=-1;
  if (delta_y>0) incy=1;
  else
    if(delta_y==0) incy=0;
    else
      incy=-1;
  delta_x=abs(delta_x);
  delta_y=abs(delta_y);
  if (delta_x>delta_y) distance=delta_x;
  else distance=delta_y;
  dp.addr=y1;
  dp.addr=dp.addr*Max_x+x1; y2=incy*Max_x;
  //page=dp.O_P.pageno-1;
  for (t=0;t<=distance+1;t++)
  {
    if (dp.O_P.pageno!=page)
    {
      page=dp.O_P.pageno;
      asm {
	mov dx,page
	mov bx,0
	mov ax,0x4f05
	int 0x10
      }
    }
    dptr.O_P.offset=dp.O_P.offset;
    *dptr.ptr=fcolor;
    xerr+=delta_x; yerr+=delta_y;
    if (xerr>distance) {xerr-=distance; x1+=incx; dp.addr+=incx;}
    if (yerr>distance) {yerr-=distance; y1+=incy; dp.addr+=y2;}
  }
}
void draw_3d(int x1,int y1,int x2,int y2,unsigned d,unsigned fc,unsigned bc)
{
  int i;
  for (i=0;i<d;i++)
  {
    fcolor=fc;
	line256(x1+i,y1+i,x2-i,y1+i);
	line256(x1+i,y1+i,x1+i,y2-i);
    fcolor=bc;
	line256(x2-i,y1+i,x2-i,y2-i);
	line256(x1+i,y2-i,x2-i,y2-i);
  }
}


//256色矩形函数
void drawbox(int x1,int y1,int x2,int y2)
{
  line256(x1,y1,x2,y1);
  line256(x2,y1,x2,y2);
  line256(x2,y2,x1,y2);
  line256(x1,y2,x1,y1);
}

//256色填充矩形函数
void fillbox(int x1,int y1,int x2,int y2)
{
  register int si,i,j;
  if (x1<0) x1=0;
  si=x1;
  if (x2>=Max_x) x2=Max_x-1;
  si+=Max_x-x2-1;
  if (y1<0) y1=0;
  if (y2>=Max_y) y2=Max_y-1;
  dp.addr=y1;
  dp.addr=dp.addr*Max_x+x1;
  page=dp.O_P.pageno-1;
  for (j=y1;j<=y2;j++,dp.addr+=si)
    for (i=x1;i<=x2;i++,dp.addr++)
    {
      if (dp.O_P.pageno!=page)
      {
	page=dp.O_P.pageno;
	asm {
	  mov dx,page
	  mov bx,0
	  mov ax,0x4f05
	  int 0x10
	}
      }
      dptr.O_P.offset=dp.O_P.offset;
      *dptr.ptr=fcolor;
    }
}

//256色图块绘制函数
void drawblock(int x,int y,int m,int n,unsigned char huge *block)
{
  int i,j,si1=0,si2=0;
  int ib=x,ie=x+m-1,jb=y,je=y+n-1;
  if (ib<0) { si1=-x;block-=x;ib=0;}
  else si2=x;
  if (ie>=Max_x) { si1+=ie-Max_x+1; ie=Max_x-1;}
  else si2+=Max_x-1-ie;
  if (jb<0) { block-=jb*m; jb=0;}
  if (je>=Max_y) je=Max_y-1;
  dp.addr=jb;
  dp.addr=dp.addr*Max_x+ib;
  //page=dp.O_P.pageno-1;
  for (j=jb;j<=je;j++,dp.addr+=si2,block+=si1)
    for (i=ib;i<=ie;i++,dp.addr++,block++)
    {
      if (dp.O_P.pageno!=page)
      {
	page=dp.O_P.pageno;
	asm {
	  mov dx,page
	  mov bx,0
	  mov ax,0x4f05
	  int 0x10
	}
      }
      dptr.O_P.offset=dp.O_P.offset;
      *dptr.ptr=*block;
    }
}

//256色图块绘制函数, 其中颜色与spccolor相同的点不作处理
void drawblockt(int x,int y,int m,int n,unsigned char huge *block)
{
  type_2 p;
  int i,j;
  int si1=0,si2=0,ib=x,ie=x+m-1,jb=y,je=y+n-1;
  unsigned char far *ptr;
  if (ib<0) { si1=-x;block-=x;ib=0;}
  else si2=x;
  if (ie>=Max_x) { si1+=ie-Max_x+1; ie=Max_x-1;}
  else si2+=Max_x-1-ie;
  if (jb<0) { block-=jb*m; jb=0;}
  if (je>=Max_y) je=Max_y-1;
  p.addr=jb;
  p.addr=p.addr*Max_x+ib;
  page=p.O_P.pageno-1;
  for (j=jb;j<=je;j++,ptr+=si2,p.addr+=si2,block+=si1)
    for (i=ib;i<=ie;i++,ptr++,p.addr++,block++)
    {
      if (*block<255)
      {
	if (p.O_P.pageno!=page)
	{
	  page=p.O_P.pageno;
	  asm {
	    mov dx,page
	    mov bx,0
	    mov ax,0x4f05
	    int 0x10
	  }
	  ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	}
	*ptr=*block;
      }
    }
}

//256色图块截取函数
void getblock(int x,int y,int m,int n,unsigned char huge *block)
{
  type_2 p;
  int i,j;
  int si1=0,si2=0,ib=x,ie=x+m-1,jb=y,je=y+n-1;
  unsigned char far *ptr;
  if (ib<0) { si1=-x;block-=x;ib=0;}
  else si2=x;
  if (ie>=Max_x) { si1+=ie-Max_x+1; ie=Max_x-1;}
  else si2+=Max_x-1-ie;
  if (jb<0) { block-=jb*m; jb=0;}
  if (je>=Max_y) je=Max_y-1;
  p.addr=jb;
  p.addr=p.addr*Max_x+ib;
  page=p.O_P.pageno-1;
  for (j=jb;j<=je;j++,ptr+=si2,p.addr+=si2)
    for (i=ib;i<=ie;i++,ptr++,p.addr++,block++)
    {
      if (p.O_P.pageno!=page)
      {
	page=p.O_P.pageno;
	asm {
	  mov dx,page
	  mov bx,0
	  mov ax,0x4f05
	  int 0x10
	}
	ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
      }
      *block=*ptr;
    }
}
void restoreblock(int x,int y,unsigned *size,unsigned char huge *block)
{
  int m,n;
  if (x<0) x=0;
  if (y<0) y=0;
  m=*size;
  size++;
  n=*size;
  drawblock(x,y,m,n,block);
}
//256色绘圆函数
/*void circle(int x_center,int y_center,int radius)
{
  int x;
  int y,delta;
  y=radius;
  delta=3-(radius<<1);
  for (x=0;x<=y;)
  {
    point(x+x_center,y+y_center);
    point(x+x_center,-y+y_center);
    point(-x+x_center,-y+y_center);
    point(-x+x_center,y+y_center);
    point(y+x_center,x+y_center);
    point(y+x_center,-x+y_center);
    point(-y+x_center,-x+y_center);
    point(-y+x_center,x+y_center);
    if (delta<0) delta+=(x<<2)+6;
    else { delta+=((x-y)<<2)+10; y--;}
    x++;
  }
}
double asp_ratio;
void plotcircle(int x,int y,int x_center,int y_center)
{
  double startx,endx,x1,starty,endy,y1;
  starty=y*asp_ratio; endy=(y+1)*asp_ratio;
  startx=x*asp_ratio; endx=(x+1)*asp_ratio;
  for (x1=startx;x1<endx;++x1)
  {
    point(x1+x_center,y+y_center);
    point(x1+x_center,-y+y_center);
    point(-x1+x_center,-y+y_center);
    point(-x1+x_center,y+y_center);
  }
  for (y1=starty;y1<endy;++y1)
  {
    point(y1+x_center,x+y_center);
    point(y1+x_center,-x+y_center);
    point(-y1+x_center,-x+y_center);
    point(-y1+x_center,x+y_center);
  }
}
void ellipse(int x_center,int y_center,int x_radius,int y_radius)
{
  register int x,y,delta;
  asp_ratio=y_radius;
  asp_ratio/=x_radius;
  y=x_radius;
  delta=3-(x_radius<<1);
  for (x=0;x<=y;)
  {
    plotcircle(x,y,x_center,y_center);
    if (delta<0) delta+=(x<<2)+6;
    else { delta+=((x-y)<<2)+10; y--;}
    x++;
  }
}*/

//测试EMS驱动程序, 找到返回1, 否则返回0
char test_ems()
{
  union REGS inregs,outregs;
  struct SREGS segs;
  struct DeviceHeader {
    DeviceHeader far *link;
    unsigned attributes;
    unsigned strategy_offset;
    unsigned interrupt_offset;
    char name_of_number_of_units[8];
  } far *dev;
  unsigned char major,minor,c[13];
  inregs.x.ax=0x3001;
  intdos(&inregs,&outregs);
  major=outregs.h.al;
  minor=outregs.h.ah;
  if (major<2) return(0);
  else
  {
     inregs.h.ah=0x52;
     intdosx(&inregs,&outregs,&segs);
     if (major==2)
       dev=(DeviceHeader far *)MK_FP(segs.es+1,outregs.x.bx+7);
     else
       if ((major==3) && (minor==0))
	 dev=(DeviceHeader far *)MK_FP(segs.es+2,outregs.x.bx+8);
       else
	 dev=(DeviceHeader far *)MK_FP(segs.es+2,outregs.x.bx+2);
     c[8]=NULL;
     while (FP_OFF(dev)!=0xffff)
     {
       if (dev->attributes & 0x8000)
       {
	 for (int i=0;i<8;i++)
	   c[i]=dev->name_of_number_of_units[i];
       }
       if (!strcmp(c,"EMMXXXX0")) return(1);
       dev=dev->link;
     }
  }
  return(0);
}
//载入16点阵英文字库
char loadenglishfont(char *dir)
{
  unsigned char buffer[4096],far *tmp;
  int asc,i;
  asc=open(dir,O_RDONLY|O_BINARY);
  read(asc,buffer,4096);
  close(asc);
  fe=(unsigned char far *)farmalloc(4096);
  if (asc==-1 || fe==NULL) return(1);
  tmp=fe;
  read(asc,buffer,4096);
  for (i=0;i<4096;i++)  *tmp++=buffer[i];
  return(0);
}
//载入16点阵汉字字库
char loadchinesefont(char *dir)
{
   int  hzk=open(dir,O_RDONLY|O_BINARY);
   char flag;
   int a,pageaddr,e;
   unsigned char far *tmp,buffer[4096];
   if (hzk!=-1)
   {
     if (test_ems())
     {
       ems=1;
       asm {
	mov ah,0x40
	int 0x67
	mov flag,ah
      }
      if (!flag)
      {
	asm {
	  mov ah,0x42
	  int 0x67
	  mov a,bx
	}
	page=28;
	if (a>=page)
	{
	  asm {
	    mov ah,0x41
	    int 0x67
	    mov flag,ah
	    mov pageaddr,bx
	  }
	}
	if (!flag)
	{
	  asm {
	    mov ah,0x43
	    mov bx,page
	    int 0x67
	    mov flag,ah
	    mov hzkems,dx
	  }
	}
	if (!flag)
	{
	  fc=(char far *)MK_FP(pageaddr,0);
	  tmp=fc+16384; a=0;
	  for (e=1;e!=0;)
	  {
	    if (tmp==fc+16384)
	    {
	      asm {
		mov ax,0x4400
		mov bx,a
		mov dx,hzkems
		int 0x67
	      }
	      a++; tmp=fc;
	    }
	    e=read(hzk,buffer,4096);
	    for (int i=0;i<e;i+=32)
	    {
	      for (int j=0;j<32;j++) *tmp++=buffer[i+j];
	    }
	  }
	  while (tmp-fc<16384) *tmp++=0;
	  close(hzk);
	}
	return(0);
      }
    }
  }
  return(1);
}
//显示16点阵英文及汉字
void showchar16(int x,int y,unsigned char *s)
{
  unsigned char far *ptr;
  unsigned char far *dot;
  unsigned p1,p2;
  type_2 p;
  int i,j,k,hzk;
  for (i=0;i<strlen(s);i++)
  {
    p.addr=y;
    p.addr=p.addr*Max_x+x;
    if ((s[i]>160) && (s[i]<255) && (s[i+1]>160))
    {
      p1=s[i]-161;
      p1=p1*94+s[i+1]-161;
      p2=p1>>9;
      asm {
	mov ax,0x4400
	mov bx,p2
	mov dx,hzkems
	int 0x67
      }
      p2=(p1 & 511)<<5;
      dot=fc+p2;
      for (j=0;j<32;j++)
      {
	for (k=0x80;k>0;k>>=1,p.addr++)
	{

	  if (k & dot[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
	if (j & 1)
	{
	  p.addr+=Max_x-16;
	}
      }
      if (showmode) y+=16+char_range;
      else x+=16+char_range;
      i++;
    }
    else
    {
      dot=fe+(s[i]<<4);
      if (showmode) p.addr+=4;
      for (j=0;j<16;j++,p.addr+=Max_x-8)
      {
	for (k=0x80;k>0;k>>=1,p.addr++)
	{
	  if (k & dot[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
      }
      if (showmode) y+=16+char_range;
      else x+=8+char_range;
    }
  }
}
//显示24点阵英文及汉字
void showchar24(int x,int y,unsigned char *s)
{
  unsigned char far *ptr;
  unsigned char buffer[72];
  long p1;
  type_2 p;
  int i,j,k,hzk,gpc,eng;
  hzk=open(cfont24,O_RDONLY|O_BINARY);
  eng=open(efont24,O_RDONLY|O_BINARY);
  gpc=open(tfont24,O_RDONLY|O_BINARY);
  for (i=0;i<strlen(s);i++)
  {
    p.addr=y;
    p.addr=p.addr*Max_x+x;
    if ((s[i]>160) && (s[i]<255) && (s[i+1]>160))
    {
      if (s[i]<0xb0)
      {
	p1=s[i]-0xa1;
	p1=p1*94+s[i+1]-0xa1;
	p1*=72;
	lseek(gpc,p1,SEEK_SET);
	read(gpc,buffer,72);
      }
      else
      {
	p1=s[i]-0xb0;
	p1=p1*94+s[i+1]-0xa1;
	p1*=72;
	lseek(hzk,p1,SEEK_SET);
	read(hzk,buffer,72);
      }
      for (j=0;j<72;j++)
      {
	for (k=0x80;k>0;k>>=1,p.addr++)
	{
	  if (k & buffer[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
	if (j%3==2)  p.addr+=Max_x-24;
      }
      if (showmode) y+=24+char_range;
      else x+=24+char_range;
      i++;
    }
    else
    {
      p1=s[i];
      p1*=48;
      lseek(eng,p1,SEEK_SET);
      read(eng,buffer,48);
      if (showmode) p.addr+=6;
      for (j=0;j<48;j++)
      {
	for (k=0x80;k>0;k>>=1,p.addr++)
	{
	  if (k & buffer[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
	j++;
	for (k=0x80;k>0x8;k>>=1,p.addr++)
	{
	  if (k & buffer[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
	p.addr+=Max_x-12;
      }
      if (showmode) y+=24+char_range;
      else x+=12+char_range;
    }
  }
  close(hzk);
  close(gpc);
  close(eng);
}
//显示32点阵英文及汉字
void showchar32(int x,int y,unsigned char *s)
{
  unsigned char buffer[32*4];
  unsigned char far *ptr;
  int i,j,k,hzk,gpc,eng;
  hzk=open(cfont32,O_RDONLY|O_BINARY);
  gpc=open(tfont32,O_RDONLY|O_BINARY);
  eng=open("asc32b.",O_RDONLY|O_BINARY);
  long p1;
  type_2 p;
  for (i=0;i<strlen(s);i++)
  {
    p.addr=y;
    p.addr=p.addr*Max_x+x;
    if ((s[i]>160) && (s[i]<255) && (s[i+1]>160))
    {
      if (s[i]<0xb0)
      {
	p1=s[i]-0xa1;
	p1=p1*94+s[i+1]-0xa1;
	p1*=(32*4);
	lseek(gpc,p1,SEEK_SET);
	read(gpc,buffer,32*4);
      }
      else
      {
	p1=s[i]-0xb0;
	p1=p1*94+s[i+1]-0xa1;
	p1*=(32*4);
	lseek(hzk,p1,SEEK_SET);
	read(hzk,buffer,32*4);
      }
      for (j=0;j<32*4;j++)
      {
	for (k=0x80;k>0;k>>=1,p.addr++)
	{
	  if (k & buffer[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
	if ((j & 3)==3) p.addr+=Max_x-32;
      }
      if (showmode) y+=32+char_range;
      else x+=32+char_range;
      i++;
    }
    else
    {
      p1=s[i]-32;
      p1*=(32*2);
      p1+=96*32*2*efont32;
      lseek(eng,p1,SEEK_SET);
      read(eng,buffer,32*2);
      p.addr+=(showmode<<3);
      for (j=0;j<32*2;j++)
      {
	for (k=0x80;k>0;k>>=1,p.addr++)
	{
	  if (k & buffer[j])
	  {
	    if (writemode & 0x80)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x70) {
		case 0x10:*ptr=fcolor;break;
		case 0x20:*ptr=bcolor;
	      }
	    }
	  }
	  else
	  {
	    if (writemode & 0x8)
	    {
	      if (page!=p.O_P.pageno)
	      {
		page=p.O_P.pageno;
		asm {
		  mov dx,page
		  mov bx,0
		  mov ax,0x4f05
		  int 0x10
		}
	      }
	      ptr=(unsigned char far *)(0xa0000000+p.O_P.offset);
	      switch (writemode & 0x7) {
		case 0x1:*ptr=fcolor;break;
		case 0x2:*ptr=bcolor;
	      }
	    }
	  }
	}
	if ((j & 1)==1)  p.addr+=Max_x-16;
      }
      if (showmode) y+=32+char_range;
      else x+=16+char_range;
    }
  }
  close(hzk);
  close(gpc);
  close(eng);
}
//显示16点阵艺术英文及汉字
//amode为 0-镶边字 1-3D字形(凸出) 2-3D字形(下凹) 3-3D字形(浮起)
void artfont16(int x,int y,char amode,unsigned char color1,unsigned char color2,char *s)
{
  int i,j;
  switch (amode) {
  case 0:
    fcolor=color2;
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
	showchar16(x+i,y+j,s);
    fcolor=color1;
    showchar16(x+1,y+1,s);
    break;
  case 1:
    fcolor=8;
    showchar16(x+2,y+2,s);
    showchar16(x+1,y+2,s);
    showchar16(x+2,y+1,s);
    fcolor=15;
    showchar16(x,y,s);
    showchar16(x+1,y,s);
    showchar16(x,y+1,s);
    fcolor=color1;
    showchar16(x+1,y+1,s);
    break;
  case 2:
    fcolor=15;
    showchar16(x+2,y+2,s);
    showchar16(x+1,y+2,s);
    showchar16(x+2,y+1,s);
    fcolor=8;
    showchar16(x,y,s);
    showchar16(x+1,y,s);
    showchar16(x,y+1,s);
    fcolor=color1;
    showchar16(x+1,y+1,s);
    break;
  case 3:
    fcolor=color2;
    showchar16(x+2,y+2,s);
    fcolor=color1;
    showchar16(x,y,s);
  }
}
//显示24点阵艺术英文及汉字
void artfont24(int x,int y,char amode,unsigned char color1,unsigned char color2,char *s)
{
  int i,j;
  switch (amode) {
  case 0:
    fcolor=color2;
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
	showchar24(x+i,y+j,s);
    fcolor=color1;
    showchar24(x+1,y+1,s);
    break;
  case 1:
    fcolor=8;
    showchar24(x+2,y+2,s);
    showchar24(x+1,y+2,s);
    showchar24(x+2,y+1,s);
    fcolor=15;
    showchar24(x,y,s);
    showchar24(x+1,y,s);
    showchar24(x,y+1,s);
    fcolor=color1;
    showchar24(x+1,y+1,s);
    break;
  case 2:
    fcolor=15;
    showchar24(x+2,y+2,s);
    showchar24(x+1,y+2,s);
    showchar24(x+2,y+1,s);
    fcolor=8;
    showchar24(x,y,s);
    showchar24(x+1,y,s);
    showchar24(x,y+1,s);
    fcolor=color1;
    showchar24(x+1,y+1,s);
    break;
  case 3:
    fcolor=color2;
    showchar24(x+2,y+2,s);
    fcolor=color1;
    showchar24(x,y,s);
  }
}
//显示32点阵艺术英文及汉字
void artfont32(int x,int y,char amode,unsigned char color1,unsigned char color2,char *s)
{
  int i,j;
  switch (amode) {
  case 0:
    fcolor=color2;
    for (i=0;i<3;i++)
      for (j=0;j<3;j++)
	showchar32(x+i,y+j,s);
    fcolor=color1;
    showchar32(x+1,y+1,s);
    break;
  case 1:
    fcolor=8;
    showchar32(x+2,y+2,s);
    showchar32(x+1,y+2,s);
    showchar32(x+2,y+1,s);
    fcolor=15;
    showchar32(x,y,s);
    showchar32(x+1,y,s);
    showchar32(x,y+1,s);
    fcolor=color1;
    showchar32(x+1,y+1,s);
    break;
  case 2:
    fcolor=15;
    showchar32(x+2,y+2,s);
    showchar32(x+1,y+2,s);
    showchar32(x+2,y+1,s);
    fcolor=8;
    showchar32(x,y,s);
    showchar32(x+1,y,s);
    showchar32(x,y+1,s);
    fcolor=color1;
    showchar32(x+1,y+1,s);
    break;
  case 3:
    fcolor=color2;
    showchar32(x+2,y+2,s);
    fcolor=color1;
    showchar32(x,y,s);
  }
}

//释放16点英文及汉字字库所分配的内存
void freefont()
{
  if (ems)
  {
  asm {
      mov ah,0x45
      mov dx,hzkems
      int 0x67
  }
  }
  farfree(fe);
}

//保存颜色寄存器值, 参数dacno为起始寄存器编号, num为保存寄存器的个数
//addr为数组指针, 容量应有num*3个字节
void savedac(int dacno,int num,unsigned char huge *addr)
{
  unsigned vseg,voff;
  vseg=FP_SEG(addr);
  voff=FP_OFF(addr);
  asm {
    push es
    mov ax,0x1017
    mov bx,dacno
    mov cx,num
    mov es,vseg
    mov dx,voff
    int 0x10
    pop es
  }
}
//恢复或改变颜色寄存器值
void restoredac(int dacno,int num,unsigned char huge *addr)
{
  unsigned vseg,voff;
  vseg=FP_SEG(addr);
  voff=FP_OFF(addr);
  asm {
    push es
    mov ax,0x1012
    mov bx,dacno
    mov cx,num
    mov es,vseg
    mov dx,voff
    int 0x10
    pop es
  }
}
//根据所给的屏幕大小及颜色位数信息设置显示模式
void set_mode(unsigned width,unsigned height,char color_bit)
{
  gmode=0x13;
  if (width>320 || height>200) gmode=0x101;
  if (width>640 || height>480) gmode=0x103;
  if (width>800 || height>600) gmode=0x105;
  switch (gmode) {
  case 0x13:
    Max_x=320,Max_y=200,Hi_Max_x=640;
    break;
  case 0x101:
    Max_x=640,Max_y=480,Hi_Max_x=1280;
    break;
  case 0x103:
    Max_x=800,Max_y=600,Hi_Max_x=1600;
    break;
  case 0x105:
    Max_x=1024,Max_y=768,Hi_Max_x=2048;
  }
  if (color_bit==24)
  {
    switch (gmode) {
    case 0x13:
      gmode=0x110;
      Max_x=640,Max_y=480,Hi_Max_x=1280;
      break;
    case 0x101:
      gmode=0x110;
      break;
    case 0x103:
      gmode=0x113;
      break;
    case 0x105:
      gmode=0x113;
      Max_x=800,Max_y=600,Hi_Max_x=1600;
    }
  }
  clearscreen();
}
void covert_pal(unsigned char (*dac)[3],unsigned char *covert)
{
  int i,j,k,n,num,s,search,con[16][2],t=0;
  long com[16],sum;
  for (i=0;i<16;i++) com[i]=0xffffff;
  for (i=0;i<255+16;i++)
    for (j=i+1;(j>15) && (j<256+16);j++)
    {
      sum=0;
      for (k=0;k<3;k++)
      {
	int tmp=dac[i][k]; tmp-=dac[j][k]; tmp*=tmp; sum+=tmp;
      }
      n=0;
      for (k=1;k<16;k++) if (com[n]<com[k]) n=k;
      if ((sum<com[n]))
      {
	for (k=0;k<16;k++) if (con[k][1]==j) break;
	if(k==16)
	{
	  com[n]=sum; con[n][0]=i; con[n][1]=j;
	}
	else
	{
	  if (sum<com[k] && com[k]==0xffffff)
	  com[k]=sum; con[k][0]=i;
	}
      }
    }
  s=16; num=0;
  for (i=0;i<256;i++)
  {
    n=0; search=i+16; t=1;
    do
    {
      if (con[n][1]==search) {search=con[n][0]; n=0; t=0;}
      else n++;
    }
    while (n<16);
    if (t)
    {
      for (k=0;k<3;k++) dac[s][k]=dac[search][k];
      for (k=0;k<16;k++)
      {
	if (con[k][1]==search) con[k][1]=s;
	if (con[k][0]==search) con[k][0]=s;
      }
	  search=s++;
	}
	covert[num++]=search;
  }
}

void outbox(int x,int y,int width,int heigth,int fc,int bc)
{
	fcolor=fc;
	line256(x,y,x+width,y);
	line256(x,y,x,y+heigth);
	fcolor=bc;
	line256(x+width,y,x+width,y+heigth);
	line256(x,y+heigth,x+width,y+heigth);
}
