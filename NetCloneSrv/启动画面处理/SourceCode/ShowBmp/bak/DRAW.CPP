//***********************************************************************
//定义几个结构，目的是为了方便操作。
typedef struct tagRGBCOLOR
{
 unsigned char red;
 unsigned char green;
 unsigned char blue;
}RGBCOLOR;
typedef struct tagBITMAPFILEHEADER //位图文件头结构
{
 unsigned int bfType;
 unsigned long int bfSize;
 unsigned int bfReserved1;
 unsigned int bfReserved2;
 unsigned long int bfOffBits;
}BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER //位图信息结构
{
 unsigned long int biSize;
 unsigned long int biWidth;
 unsigned long int biHeight;
 unsigned int biPlanes;
 unsigned int bibitCount;
 unsigned long int biCompression;
 unsigned long int biSizeImage;
 unsigned long int biXPelsPerMeter;
 unsigned long int biYPelsPerMeter;
 unsigned long int biClrUsed;
 unsigned long int biClrImportant;
}BITMAPINFOHEADER;


void GetPalette(int index,RGBCOLOR *color) //这是通过索引获取调色版的值的函数
{
 outportb(0x3c6,0xff);
 outportb(0x3c7,index);
 color->red=inportb(0x3c9);
 color->green=inportb(0x3c9);
 color->blue=inportb(0x3c9);
}

void SetPalette(int index,RGBCOLOR *color) //设置调色版寄存器的函数
{
 outportb(0x3c6,0xff);
 outportb(0x3c8,index);
 outportb(0x3c9,color->red*63/0xff);
 outportb(0x3c9,color->green*63/0xff);
 outportb(0x3c9,color->blue*63/0xff);
}

//**********************************************
//画位图的函数
/*参数说明:
*fp 是文件指针
X 目标X
Y 目标Y
SrcX 源X坐标
SrcY 源Y坐标
Width 源的宽
Height 源的高
HideColorIndex 要屏蔽的Index号，可通过这个值使位图去掉背景色，不想去掉背景色的可设为-1
*/
int DrawImage(FILE *fp,int X=0,int Y=0,
  int SrcX=0,int SrcY=0,int Width=0,int Height=0,
  int HideColorIndex=-1)
{
 char *lpData; //数据指针
 unsigned int fsize; //文件大小
 unsigned int w,bh,bs; //w为已对齐的宽度，bh为位图的高度，bs为已经对齐的宽*原高
 char far *VRBuf=(char far*)0xa0000000; //指向显存的远指针
 BITMAPINFOHEADER bih;
 RGBCOLOR *rgbcolor;
 fseek(fp,2,SEEK_SET); //移动文件指针，指向文件头结构的bfSize字段
 fread(&fsize,2,1,fp); //读整个位图文件的大小
 lpData=(char *)malloc(fsize); //为位图分配内存，使lpData指针指向该内存
 if(lpData==NULL)
 {
  cout<<"alloc error"<<endl;
  return 0;
 }
 fseek(fp,0,SEEK_SET);
 fread(lpData,fsize,1,fp); //将整个文件读进由lpData指向的内存区
 memcpy(&bih,lpData+sizeof(BITMAPFILEHEADER),sizeof(BITMAPINFOHEADER)); //读位图信息结构的内容到结构bih
 if(Width>bih.biWidth||Height>bih.biHeight) //如果参数Width和Height过大则显示错误信息和返回
 {
  cout<<"Width or Height is error"<<endl;
  free(lpData);
  return 0;
 }
 if(Width==0) //如果参数Width和Height为0则设置为原来大小
  Width=bih.biWidth;
 if(Height==0)
  Height=bih.biHeight;
 bh=bih.biHeight;
 bs=bih.biSizeImage;
 lpData=lpData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); //使lpData指向调色版数据
 //**********************************
 //这里是设置调色版寄存器
 int i,j;
 for(i=0;i<256;i++)
 {
  rgbcolor->blue=*lpData;
  rgbcolor->green=*(lpData+1);
  rgbcolor->red=*(lpData+2);
  lpData+=4;
  SetPalette(i,rgbcolor);
 }
 //**********************************
 
 //将位图数据复制到显存，现在lpData已经指向了位图数据区
 //这里我写得可能有点复杂，请读者慢慢领悟吧
 w=bs/bh; //已经对齐的宽度
 lpData+=bs-w*(SrcY+1)+SrcX; //因为位图文件是倒过来存放的，所以要做处理
 VRBuf+=Y*320+X; //要实现画到屏幕X，Y坐标，这样就可以了
 for(i=0;i<Height;i++) //只提取Height参数的高度
 {
  for(j=0;j<Width;j++) //只提取Width参数的宽度
  {
   if((char)HideColorIndex!=*(lpData+j)) //屏蔽背景色
      *(VRBuf+j)=*(lpData+j);
  }
  VRBuf+=320; //换行
  lpData-=w; //换行
 }
 free(lpData); //释放内存
 return 1; //返回1表示成功
}
//***********************************************************************
