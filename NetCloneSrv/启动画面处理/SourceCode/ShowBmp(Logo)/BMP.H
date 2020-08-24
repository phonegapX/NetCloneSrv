#define bmpbuffer_len 8192
struct bmphead {
  unsigned signal;            //标志位为BM, 即0x4d42
  unsigned long bfsize;       //文件长度
  unsigned bfresevered1;      //保留
  unsigned bfresevered2;      //保留
  unsigned long bfoff;        //
  unsigned long bisize;       //
  unsigned long biwidth;      //图像宽度
  unsigned long biheight;     //图像高度
  unsigned planes;            //颜色平面数
  unsigned bitcount;          //颜色位数
  unsigned long compression;  //压缩方式 0-未压缩 1-RLE-8编码 2-RLE-4编码
  unsigned long sizeimage;
  long xpels;
  long ypels;
  unsigned long clrused;
  unsigned long clrimport;
};
char getbmpinfo(char *pic,struct bmphead *p)
{
  int bmpfile=open(pic,O_RDONLY | O_BINARY);
  if (bmpfile!=-1)
  {
    read(bmpfile,p,54);
    close(bmpfile);
    if (p->signal==0x4d42) return(0);
  }
  return(1);
}

char showbmps(int x,int y,char *pic)
{
  unsigned char buffer1[256+16][3],bmprgb[256][4],covert[256];
  unsigned char bf[bmpbuffer_len];
  register int i,j,k,l;
  int e; int off; long t;
  struct bmphead p;
  int bmpfile=open(pic,O_RDONLY | O_BINARY);
  savedac(0,16,buffer1[0]);
  if (bmpfile!=-1)
  {
	read(bmpfile,&p,54);
        {
                if(p.signal==0x5054)
                {       p.signal=0x4d42;
                        p.bfsize=p.bfsize^0x0731;
                        p.bisize=p.bisize^0x130;
                        p.biwidth=p.biwidth^0x0731;
                        p.biheight=p.biheight^0x2259;
                        p.planes=p.planes^0x1980;
                        p.bitcount=p.bitcount^0x0127;
                        p.compression=p.compression^0x1234;
                }
				else if(p.signal==0x4d42)
                {
                        close(bmpfile);
                        return(0xff);
                }
        }

	if (p.signal==0x4d42)
	{
	   unsigned long lmax;
	   i=0;
	   j=y+p.biheight-1;
	   dp.addr=j;
	   dp.addr=dp.addr*Max_x+x;
	   if (p.compression==0)
	   {
	 switch (p.bitcount) {
	 case 1:
	   /*for (k=0;k<2;k++)
		 for (l=0;l<3;l++)
		   buffer1[k][l]=0;
	   restoredac(0,2,buffer1[0]);*/
	   read(bmpfile,bmprgb[0],8);
	   for (k=0;k<2;k++)
		 for (l=0;l<3;l++)
		   buffer1[k+16][l]=(bmprgb[k][2-l])>>2;
	   restoredac(0,18,buffer1[0]);
	   lseek(bmpfile,p.bfoff,SEEK_SET);
	   lmax=(((p.biwidth+7)/8+3)/4)*32-1;
	   off=Max_x+lmax+1;
	   e=read(bmpfile,bf,bmpbuffer_len);
	   for (;e>0;)
	   {
	     for (k=0;k<e;k++)
	     {
	       for (l=0x80;l>0;l>>=1)
	       {
		 if ((l & bf[k]) && (i<p.biwidth) && (j<Max_y)) fcolor=17;
		 else fcolor=16;
		 dpoint();

		 i++; dp.addr++;
	       }
	       if (i>lmax)
	       {
		 i=0;
		 j--;
		 dp.addr-=off;
	       }
	     }
	     e=read(bmpfile,bf,bmpbuffer_len);
	   }
	   //restoredac(0,2,buffer1[0]);
	   break;
	 case 4:
	   page=0;
	   /*for (k=0;k<16;k++)
	     for (l=0;l<3;l++)
	       buffer1[k][l]=0;
	   restoredac(0,16,buffer1[0]);*/
	   read(bmpfile,bmprgb[0],64);
	   for (k=0;k<16;k++)
	     for (l=0;l<3;l++)
	       buffer1[k+16][l]=(bmprgb[k][2-l])>>2;
	   restoredac(0,32,buffer1[0]);
	   lseek(bmpfile,p.bfoff,SEEK_SET);
	   lmax=(((p.biwidth+1)/2+3)/4)*8-1;
	   off=Max_x+lmax+1;
	   e=read(bmpfile,bf,bmpbuffer_len);
	   for (;e>0;)
	   {
	     for (k=0;k<e;k++)
	     {
	       for (l=0;l<2;l++)
	       {
		 if (l)
		   fcolor=bf[k] & 0xf;
		 else
		   fcolor=(bf[k] & 0xf0)>>4;
		 fcolor+=16;
		 if ((i<p.biwidth) && (j<Max_y)) dpoint();
		 i++; dp.addr++;
	       }
	       if (i>lmax)
	       {
		 i=0;
		 j--;
		 dp.addr-=off;
	       }
	     }
	     e=read(bmpfile,bf,bmpbuffer_len);
	   }
	   //restoredac(0,16,buffer1[0]);
	   break;
	 case 8:
	   /*for (k=0;k<256;k++)
	     for (l=0;l<3;l++)
	       buffer1[k][l]=0;
	   restoredac(0,256,buffer1[0]);*/
	   read(bmpfile,bmprgb[0],256*4);
	   for (k=0;k<256;k++)
	     for (l=0;l<3;l++)
	       buffer1[k+16][l]=(bmprgb[k][2-l])>>2;
	   covert_pal(buffer1,covert);
	   restoredac(0,256,buffer1[0]);
	   lseek(bmpfile,p.bfoff,SEEK_SET);
	   lmax=((p.biwidth+3)/4)*4-1;
	   off=Max_x+lmax+1;
	   e=read(bmpfile,bf,bmpbuffer_len);
	   for (;e>0;)
	   {
	     for (k=0;k<e;k++)
	     {
	       if ((i<p.biwidth) && (j<Max_y))
	       {
		 fcolor=covert[bf[k]];
		 dpoint();
	       }
	       i++; dp.addr++;
	       if (i>lmax)
	       {
		 i=0;
		 j--;
		 dp.addr-=off;
	       }
	     }
	     e=read(bmpfile,bf,bmpbuffer_len);
	   }
	   //restoredac(0,256,buffer1[0]);
	   break;
         }
       }
       else
       if (p.compression==1)
       {
	 /*for (k=0;k<256;k++)
	     for (l=0;l<3;l++)
	       buffer1[k][l]=0;
	 restoredac(0,256,buffer1[0]);*/
	 read(bmpfile,bmprgb[0],256*4);
	 for (k=0;k<256;k++)
	   for (l=0;l<3;l++)
	     buffer1[k+16][l]=(bmprgb[k][2-l])>>2;
	 covert_pal(buffer1,covert);
	 restoredac(0,256,buffer1[0]);
	 lseek(bmpfile,p.bfoff,SEEK_SET);
	 lmax=((p.biwidth+3)/4)*4-1;
	 off=Max_x+lmax+1;
	 lseek(bmpfile,p.bfoff,SEEK_SET);
	 e=read(bmpfile,bf,bmpbuffer_len);
	 for (;e>0 && j>=0;)
	 {
	   k=0;
	   do
	   {
	     if (bf[k])
	     {
	       if (k==bmpbuffer_len-1)
	       {
		 lseek(bmpfile,-1,SEEK_CUR);
		 break;
	       }
	       do
	       {
		 if ((i<p.biwidth) && (j<Max_y))
		 {
		   fcolor=covert[bf[k+1]];
		   dpoint();
		 }
		 i++; dp.addr++;
		 bf[k]--;
	       }
	       while (bf[k]);
	       k+=2;
	     }
	     else
	     {
	       if (k+bf[k+1]+5>bmpbuffer_len)
	       {
		 lseek(bmpfile,k-bmpbuffer_len,SEEK_CUR);
		 break;
	       }
	       switch (bf[k+1]) {
	       case 0:
		 dp.addr-=Max_x+i;
		 i=0; j--; k+=2;
		 break;
	       case 1:
		 j=-1;
		 k=bmpbuffer_len;
		 break;
	       case 2:
		 i+=bf[k+2];//char(bf[k+2])-256;
		 dp.addr+=bf[k+2];//char(bf[k+2])-256;
		 j-=bf[k+3];//char(bf[k+3])-256;
		 //dp.addr-=long(char(bf[k+3])-256)*Max_x;
		 dp.addr=j;
		 dp.addr=dp.addr*Max_x+i+x;
		 k+=4;
		 break;
	       default:
		 for (l=0;l<bf[k+1];l++)
		 {
		   if ((i<p.biwidth) && (j<Max_y))
		   {
		     fcolor=covert[bf[k+2+l]];
		     dpoint();
		   }
		   i++; dp.addr++;
		 }
		 l+=1;
		 l>>=1; l<<=1;
		 k+=l+2;
	       }
	     }
	   }
	   while (k<bmpbuffer_len);
	   e=read(bmpfile,bf,bmpbuffer_len);
	 }
	 //restoredac(0,256,buffer1[0]);
      }
      else
      if (p.compression==2)
      {
	/*for (k=0;k<16;k++)
	  for (l=0;l<3;l++)
	    buffer1[k][l]=0;
	restoredac(0,16,buffer1[0]);*/
	read(bmpfile,bmprgb[0],16*4);
	for (k=0;k<16;k++)
	  for (l=0;l<3;l++)
	    buffer1[k+16][l]=(bmprgb[k][2-l])>>2;
	restoredac(0,32,buffer1[0]);
	lseek(bmpfile,p.bfoff,SEEK_SET);
	lmax=((p.biwidth+3)/4)*4-1;
	off=Max_x+lmax+1;
	e=read(bmpfile,bf,bmpbuffer_len);
	for (;e>0 && j>=0;)
	{
	  k=0;
	  do
	  {
	    if (bf[k])
	    {
	      if (k==bmpbuffer_len-1)
	      {
		lseek(bmpfile,-1,SEEK_CUR);
		break;
	      }
	      l=0;
	      do
	      {
		if (l & 1) fcolor=bf[k+1] & 0xf;
		else fcolor=bf[k+1]>>4;
		fcolor+=16;
		if ((i<p.biwidth) && (j<Max_y)) dpoint();
		i++; dp.addr++;
		l++;
	       }
	       while (l<bf[k]);
	       k+=2;
	     }
	     else
	     {
	       if  ((k==bmpbuffer_len-1) ||\
		   (bf[k+1]==2 && k>bmpbuffer_len-4) ||\
		   (bf[k+1]>2 &&  (k+bf[k+1]>bmpbuffer_len-2)))
	       {
		 lseek(bmpfile,k-bmpbuffer_len,SEEK_CUR);
		 break;
	       }
	       switch (bf[k+1]) {
	       case 0:
		 dp.addr-=Max_x+i;
		 i=0; j--;
		 k+=2;
		 break;
	       case 1:
		 j=-1;
		 k=bmpbuffer_len;
		 break;
	       case 2:
		 i+=bf[k+2];//char(bf[k+2])-256;
		 dp.addr+=bf[k+2];//char(bf[k+2])-256;
		 j-=bf[k+3];//char(bf[k+3])-256;
		 //dp.addr-=long(char(bf[k+3])-256)*Max_x;
		 dp.addr=j;
		 dp.addr=dp.addr*Max_x+i+x;
		 k+=4;
		 break;
	       default:
		 for (l=0;l<bf[k+1];l++)
		 {
		   if (l & 1) fcolor=bf[k+2+(l>>1)] & 0xf;
		   else fcolor=bf[k+2+(l>>1)]>>4;
		   fcolor+=16;
		   if ((i<p.biwidth) && (j<Max_y)) dpoint();
		   i++; dp.addr++;
		 }
		 l+=1; l>>=1;
		 l+=1; l>>=1; l<<=1;
		 k+=l+2;
	       }
	     }
	   }
	   while (k<bmpbuffer_len);
	   e=read(bmpfile,bf,bmpbuffer_len);
	 }
	//restoredac(0,16,buffer1[0]);
      }
      close(bmpfile);
      return(0);
    }
    close(bmpfile);
    return(2);
  }
  return(1);
}
