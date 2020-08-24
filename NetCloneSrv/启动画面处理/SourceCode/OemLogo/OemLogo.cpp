#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>

#define TABLESIZES 8192
#define LZWMAXCODES 4096
#define LZWMAXBITS 12

typedef struct {
  unsigned char *blockBuf;
  unsigned *encodeTable;
  unsigned *prefixTable;
  unsigned *suffixTable;
  unsigned initBits;
  unsigned clearCode;
  unsigned eofCode;
  unsigned encode;
  unsigned runBits;
  unsigned maxCodeSize;
  unsigned byteCount;
  unsigned shiftBits;
  unsigned long tempCode;
} LZWencodeType;

LZWencodeType LZWcode;
unsigned revalue=0;

unsigned saveCode(FILE *fp){
	unsigned result=0;
	if(fwrite(LZWcode.blockBuf,1,LZWcode.byteCount,fp)!=LZWcode.byteCount)result=1;
	LZWcode.byteCount=0;
	return(result);
}

unsigned fillBlockBuf(FILE *fp,int code)
{
	LZWcode.tempCode|=(unsigned long)code<<LZWcode.shiftBits;
	LZWcode.shiftBits+=LZWcode.runBits;
	while(LZWcode.shiftBits>=8){
		LZWcode.blockBuf[LZWcode.byteCount++]=LZWcode.tempCode & 0x00ff;
		if(LZWcode.byteCount==256)revalue=saveCode(fp);
		LZWcode.tempCode>>=8;
		LZWcode.shiftBits-=8;
	}
	return (revalue);
}

//void main (int argc,char *argv[]) {
void Lzw(unsigned char * InFileName,unsigned char * OutFileName)
{
	FILE *fin,*fout;
	unsigned colorBits=8;
	unsigned long fsize;
	unsigned char *byteBuf;
	unsigned prefixCode,suffixCode,m,n,db,flag,index,i,count,value,cs;

	fin=fopen(InFileName,"rb");
	if(fin==NULL)return;
	fout=fopen(OutFileName,"wb");
	if(fout==NULL)return;

	fseek(fin,0L,SEEK_END);
	fsize=ftell(fin);
	fseek(fin,0L,SEEK_SET);
	m=fsize/1024;
	n=fsize%1024;

	LZWcode.blockBuf=(unsigned char *)malloc(256);if(LZWcode.blockBuf==NULL)return;
	byteBuf=(unsigned char*)malloc(1024);if(byteBuf==NULL){printf("Malloc memory fail.\n");return;}
	LZWcode.encodeTable=(unsigned int *)malloc(TABLESIZES*2);if(LZWcode.encodeTable==NULL){printf("Malloc memory fail.\n");return;}
	LZWcode.prefixTable=(unsigned int *)malloc(TABLESIZES*2);if(LZWcode.prefixTable==NULL)return;
	LZWcode.suffixTable=(unsigned int *)malloc(TABLESIZES*2);if(LZWcode.suffixTable==NULL)return;

	LZWcode.initBits=colorBits;
	LZWcode.clearCode=1<<colorBits;
	LZWcode.eofCode=LZWcode.clearCode+1;
	LZWcode.byteCount=LZWcode.shiftBits=0;
	LZWcode.tempCode=0;
	LZWcode.encode=LZWcode.eofCode+1;
	LZWcode.runBits=LZWcode.initBits+1;
	LZWcode.maxCodeSize=1<<LZWcode.runBits;
	for(i=0;i<TABLESIZES;i++)LZWcode.encodeTable[i]=0;

	fillBlockBuf(fout,LZWcode.clearCode);

	for(count=0;count<m+1;count++){
		if(count==m)flag=n;	else flag=1024;
		fread(byteBuf,flag,1,fin);
		if(count==0){
			prefixCode=byteBuf[0];
			db=1;
		}else db=0;

		while(db<flag){
			suffixCode=byteBuf[db++];
			index=prefixCode^(suffixCode<<4);cs=0;
			loop:if(LZWcode.encodeTable[index]==0){
				fillBlockBuf(fout,prefixCode);
				if(LZWcode.encode==LZWMAXCODES){
					fillBlockBuf(fout,LZWcode.clearCode);
					LZWcode.encode=LZWcode.eofCode+1;
					LZWcode.runBits=LZWcode.initBits+1;
					LZWcode.maxCodeSize=1<<LZWcode.runBits;
					for(i=0;i<TABLESIZES;i++)LZWcode.encodeTable[i]=0;
				}else{
					if(LZWcode.encode==LZWcode.maxCodeSize){
						LZWcode.maxCodeSize<<=1;
						LZWcode.runBits++;
					}
					LZWcode.prefixTable[index]=prefixCode;
					LZWcode.suffixTable[index]=suffixCode;
					LZWcode.encodeTable[index]=LZWcode.encode++;
				}
				prefixCode=suffixCode;
				continue;
			}
			if(LZWcode.prefixTable[index]==prefixCode && LZWcode.suffixTable[index]==suffixCode){
				prefixCode=LZWcode.encodeTable[index];
				continue;
			}else{
				cs++;
				index=TABLESIZES-cs;
				goto loop;
			}
		}
	}
	if( !(revalue=fillBlockBuf(fout,prefixCode)) && !(revalue=fillBlockBuf(fout,LZWcode.eofCode)) ){
		if(LZWcode.shiftBits>0 || LZWcode.byteCount>0){
			LZWcode.blockBuf[LZWcode.byteCount++]=LZWcode.tempCode & 0x00ff;
			saveCode(fout);
		}
	}

	free(LZWcode.suffixTable);
	free(LZWcode.prefixTable);
	free(LZWcode.encodeTable);
	free(byteBuf);
	free(LZWcode.blockBuf);
	fclose(fout);
	fclose(fin);
}

void main(int argc,char * argv[])
{
	if(argc!=3)return;
	Lzw(argv[1],argv[2]);
	printf("Compress success.\n");
}

