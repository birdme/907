/*===================================================*/
/*程序名称:hddeal.c  话单分拣文件              */
 /*MsfotX3000 话单处理  */
/*===================================================*/

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>


#define eNOERR		"00"	

typedef struct		//详单结构体
{
	int	csn;										//话单序列号-交换机产生  pos=0 len=4 数字
  //	char	billtype[4];					//话单类型 0x01： 详细话单；0x55： 失败话单；0xFF： 告警话单  pos=7 len=1 数字16
  int Partialflag;        		//0：单个记录1：记录的第一部分 2：记录的中间部分 3：记录的最后部分 4：有部分记录的总的记录  (lo hi) pos=9(li) len=0.5 bin
	int validflag;
	int	freeflag;							//1：计费  0：免费   pos=9.75 len=0.125 bin
	int termination;           //pos=87 len=0.5  add by liudq@20171014
	int chargeparty;          //pos=10.5 len=0.5   add by liudq@20171014
	
	char	callingphone[32];			//pos=30 len=16 bcd
	char	calledphone[32];			//pos=49 len=16 bcd
	char	dialnumber[32];			//pos=139 len=16  拨打号码
	char	begintime[12];				//pos=11 len=6  bcd
	char	endtime[12];					//pos=17 len=6  bcd
	char	callingareacode[10];
	char	calledareacode[10];
	char centerareacode[10];
	int		duration;							//pos=23 len=4  数字
	int 	feedur;
	int	timelong;						//时长
	int fee;								//费用
	int	intrunks;					//pos=77 len=2  数字
	int	outtrunks;				//pos=79 len=2  数字
	int inTeleId;					//入中继运营商标识
	int outTeleId;				//出中继运营商标志
	int calltype;					//呼叫类型
	int innerCalltype;		//内部呼叫类型
	int custId;					//客户ID
	int roamflag;				//漫游标志
	char	sourcefilename[128];
	char	errno[8];
	char	billingcycleid[9];
	int   byear;
	int   bmonth;
	int		bday;
	int   eyear;
	int   emonth;					//月
	int		eday;						//日
}Latn,*pLatn;

typedef struct  
{
	char  flag[18];		//标志
	int		lenpack;		//包的长度
	int   sposbilltype;  //话单类型
  int		lenbilltype;			//
	int		sposcsn;				//csn号开始位置
	int		lencsn;				//csn长度
	int		sposPFflag;				//Partial、free开始位置
	int		lenPFflag;				//Partial、free长度
	int		sposcalling;	        //主叫开始位置
	int		lencalling;		//主叫长度
	int		sposcalled;		//被叫开始位置
	int		lencalled;		//被叫长度
	int		sposdn;						//拨叫号码开始位置
	int		lendn;						//拨叫号码长度
	int		sposbdate;		//开始日期开始位置
	int		lenbdate;		//开始长度
	int		sposedate;		//结束日期开始位置
	int		lenedate;		//结束长度
	int		sposdur;		//时长开始位置
	int		lendur;		//时长长度
	int		sposintrunks;		//入中继开始位置
	int		lenintrunks;		//入中继长度
	int		sposouttrunks;		//出中继开始位置
	int		lenouttrunks;		//出中继长度
	int		spostermination;  //话终原因码位置
	int   lentermination;  //话终原因码长度
	int 	sposchargeparty;  //计费方身份识别位置
	int   lenchargeparty;   //计费方身份识别长度

}Hwinfo,*pHwinfo;


typedef struct
{
	char filename[128];
	char dealtime[32];
	int tableid;
	int size;
	int count;
	int minCsn;
	int maxCsn;
	char target[128];
}Fileinfo,*pFileinfo;

typedef struct {
	int telecomId;
	int trunkId;
} trunks,*ptrunks;

//struct trunks trunk[trunksNumber];

typedef struct {
	trunks *phead;
	int len;
	int cnt;
}TrunkInfo,*ptrunkInfo;


typedef struct {
	int hcode;
	char areacode[8];
} hcode,*phcode;

//struct trunks trunk[trunksNumber];

typedef struct {
	hcode *phchead;
	int len;
	int cnt;
}HcodeInfo,*phcodeInfo;

typedef struct {
	int hcodeseg;
	int from;
	int to;
} hcodeindex,*phcodeindex;

//struct trunks trunk[trunksNumber];

typedef struct {
	hcodeindex *phcihead;
	int len;
	int cnt;
}HcodeindexInfo,*phcodeindexInfo;



typedef struct {
	int centerNum;
	char areacode[8];
} cenNum,*pcenNum;


typedef struct {
	cenNum *pcnhead;
	int len;
	int cnt;
}CenNumInfo,*pcennumInfo;




char *trim(char *src){
	int i,Len;
	char *ls_temp;
	Len=strlen(src);
	for(i=Len-1;i>0;i--)
	{
		if((src[i]==' ')||(src[i]=='\t')||(src[i]=='\r')||(src[i]=='\n') )
			{
				src[i]='\0';
			}
			else{
				break;
			}
	}
	ls_temp=src;
	for(i=0;i<strlen(src);i++)
	{
		if((src[i]==' ')||(src[i]=='\t'))
			{
				ls_temp++;
			}
			else{
				break;
			}
	}
	strcpy(src,ls_temp);
	return src;
	
}

char *deleZero(char *src){
	char *ls_temp;
	ls_temp=src;	
	if (src[0]=='0'){
		ls_temp++;
	}
	strcpy(src,ls_temp);
	return src;
}

/*从字符串的右边截取n个字符*/  
char * delePrex(char *src, int n)  
{  
   char *ls_temp;
   int i=0;
		ls_temp=src;	
   for(i=0;i<n;i++){
		ls_temp++;
		}
	strcpy(src,ls_temp);
	return src;
}  


int getPhonetype(char * number){
	int len;
	len=strlen(number);
	char seg[8];
	
	memset(seg,'\0',sizeof(seg));
	if (len<11){
		return 5;         //不足11的，不取区号
	}
	
	if (len>16){
		return 5;					//超过12位的，也不取区号
	}
	
	if(strncmp("888888888888",number,8)==0){
		return 5;	
	}
	
	if (number[0]=='0'){
		
		if (strncmp("0400",number,4)==0){
				return 5;
			}
			
		if (strncmp("00",number,2)==0){
				return 5;
			}
		if (strncmp(number,"01",2)==0){
			if (strncmp(number,"010",3)==0){
				return 1;			//固话
			}
	
			strcpy(number,deleZero(number));
			
			strncpy(seg,number,3);
			if(atoi(seg)>=130 && atoi(seg)<=189 && atoi(seg)/10 !=16){
				return 2;			//手机
			}
			return 5;       //01开头的乱码不取区号 
				
		}
		return 1;					//固话
	}
	
	if(number[0]=='1'){
		strncpy(seg,number,3);
		if(atoi(seg)>=130 && atoi(seg)<=189 && atoi(seg)/10 !=16){
			return 2;			//手机
		}
		return 5;			//1开头的乱码不取区号 
	}
	
	return 1;				//固话
}

//custid
/*
typedef struct {
	char custId[4];
	int cmplen;
	int numberhead;
}custcalled,*pcustcalled;

typedef struct {
	custcalled *pcchead;
	int len;
	int cnt;	
}CcalledInfo,*pccalledInfo;


void initCcalledInfo(CclledInfo *pccInfo,int length){
	pccInfo->pcchead=(custcalled *)malloc(sizeof(custcalled)*length);
	if(NULL==pccInfo->pcchead){
		printf("initCcalledInfo memeory out!\n");
			exit(-1);
	}else
	{
		pccInfo->len=length;
		pccInfo->cnt=0;
	}
}

int appendCcalled(CclledInfo *pccInfo,char *cId,int cmpl,int nhead,int i){

		memset(pccInfo->pcchead[pccInfo->cnt].custId,'\0',sizeof(pccInfo->pcchead[pccInfo->cnt].custId));
		sprintf(pccInfo->pcchead[pccInfo->cnt].custId,%s,cId);
		pccInfo->pcchead[ptccInfo->cnt].cmplen=cmpl;
		pccInfo->pcchead[ptccInfo->cnt].numberhead=nhead
		ptccInfo->cnt=i;
		return 1;
	
	}  */

void initTrunkInfo(TrunkInfo *ptInfo,int length){
		ptInfo->phead=(trunks *)malloc(sizeof(trunks)*length);

		if(NULL==ptInfo->phead)
		{
			printf("initTrunkInfo memeory out!\n");
			exit(-1);
		}else 
		{
				ptInfo->len=length;
				ptInfo->cnt=0;
		}
	
	}

int isEmpty(TrunkInfo *ptInfo)
{
	if(0==ptInfo->cnt){
		return 1;
	}else{
		return 0;
	}
}
	
int isFull(TrunkInfo *ptInfo){
	if (ptInfo->len==ptInfo->cnt){
		return 1;							//满了返回1
	}else
	{
			return 0;						//不满返回0
	}
	
}

int appendTrunks(TrunkInfo *ptInfo,int teleId,int trId,int i){
		if(isFull(ptInfo)){
			printf("Trunks is full\n");
			return -1;
		}
		ptInfo->phead[ptInfo->cnt].telecomId=teleId;
		ptInfo->phead[ptInfo->cnt].trunkId=trId;
		ptInfo->cnt=i;
		return 1;
	
	}


void printTrunks(TrunkInfo *ptInfo)
{
	
	int i;
	/*if(isEmpty(&ptInfo)){
		printf("Trunks is empty\n");
	}else
	{
		for(i=0;i<ptInfo->cnt;i++){
			printf("local=%s\n %d telecomid=%d,trunkid=%d\n",ptInfo->trunkLocal,ptInfo->phead[i].telecomId,ptInfo->phead[i].trunkId);
		}
	}*/
	
	for(i=0;i<ptInfo->len;i++){
			printf("%d telecomid=%d,trunkid=%d\n",i,ptInfo->phead[i].telecomId,ptInfo->phead[i].trunkId);
		}

}

int getTelecomId(TrunkInfo *ptInfo,int trId)
{
	int low=0;
	int high=ptInfo->len;
	int mid=0;
	
	while(low<=high){
		mid=(low+high)/2;
		if(ptInfo->phead[mid].trunkId==trId){
			 return ptInfo->phead[mid].telecomId;
			}
		if(ptInfo->phead[mid].trunkId<trId) 
			{low=mid+1;
			}else
			{
					high=mid-1;
			}
	}
	return -1;
	
}




int getTrunkLen(char* local){
	FILE *fp=NULL;
	char filename[32];	
	char buf[64];
	char s[64];
	char temp[64];
	char ch;
	int len;
	char *delim;
	char *p;
	memset(filename,'\0',sizeof(filename));
	sprintf(filename,"./trunks_%s.ini",local);
	int le=0;
	
	fp=fopen(filename,"r");
	if(fp==NULL){
		printf ("open file %s erro\n",filename);
		return 0;
	}
	
	while(!feof(fp)){
		if(fgets(buf,sizeof(buf),fp)!=NULL){
	 				strcpy(s,trim(buf));
	 				
	 				//printf("read is %s\n",s);
	 				ch=buf[0];
	 				if(ch=='#' ||ch==' '||  ch=='\n'||ch=='\0')
	 					continue;
	 				
	 				if (ch==';'){
	 						 	delim="=";
	 						 	p=strtok(s,delim);
	 						 	
	 						 	
	 						 	if(strncmp(";length",p,7)==0){
	 						 		len=strlen(s);
          				memset(temp,0,sizeof(temp));
         	 				sprintf(temp,s+strlen(p)+1);
          				memset(s,0,sizeof(s));
          				sprintf(s,trim(temp));
           				//printf("scend is %s",s);
           				le=atoi(s);
           				//printf("length=%d\n",length);
           				fclose(fp);
           				return le;
           				
	 						 	}else{
	 						 		continue;
	 						 	}		
           			
	 				}	else {
	 						continue;
	 				}
	 				
		}
		fclose(fp);
		break;
		return le;
	}
	return le;
		
}


int readTrunks(TrunkInfo ptInfo,int length){
	
	FILE *fp=NULL;
	char filename[32];
	int readflag=0;
	
	char buf[64];
	char s[64];
	
	char *delim;
	char *p;
	char ch;
	int len;
	char temp[64];
	int i=1;
	int teleId=0;
	int trId=0;

	

	memset(filename,'\0',sizeof(filename));
	sprintf(filename,"%s","./trunks.ini");
	fp=fopen(filename,"r");
	if (NULL==fp){
		printf("open file %s is failture\n",filename);	
		return -1;
	}
	

	
	 while (!feof(fp))
	 {
	 	
	 		if(fgets(buf,sizeof(buf),fp)!=NULL){
	 				strcpy(s,trim(buf));
	 				
	 				ch=buf[0];
	 				if(ch=='#' ||ch==' '||  ch=='\n'||ch=='\0' ||ch==';')
	 					continue;
	 				
	 				if (ch=='('){
	 						delim=",";
	 						readflag=1;
	 						continue;
	 				}
	 				
	 				
	 				if (ch==')'){
	 					readflag=0;
	 					break;
	 				}
	 				
	 				p=strtok(s,delim);
	 				
	 				//printf("first is %s ",p);
	 				
	 				if (readflag==1){
	 					teleId=atoi(p);
          	len=strlen(s);
          	memset(temp,0,sizeof(temp));
         	 	sprintf(temp,s+strlen(p)+1);
          	memset(s,0,sizeof(s));
          	sprintf(s,temp);
           	//printf("scend is %s\n",s);
           	trId=atoi(s);
               // p=strtok(s,delim);
            appendTrunks(&ptInfo,teleId,trId,i);
            i++;
	 				}	
	 				//}
	 					 					
	 		}	
	 	} 
	 		return 1;	
	}
	

void initHcodeInfo(HcodeInfo *phcInfo,int length){
		phcInfo->phchead=(hcode *)malloc(sizeof(hcode)*length);
		if(NULL==phcInfo->phchead)
		{
			printf("initHcodeInfo memeory out!\n");
			exit(-1);
		}else 
		{
				phcInfo->len=length;
				phcInfo->cnt=0;
		}
	
	}

int isHcodeEmpty(HcodeInfo *phcInfo)
{
	if(0==phcInfo->cnt){
		return 1;
	}else{
		return 0;
	}
}
	
int isHcodeFull(HcodeInfo *phcInfo){
	if (phcInfo->len==phcInfo->cnt){
		return 1;							//满了返回1
	}else
	{
			return 0;						//不满返回0
	}
	
}

int appendHcode(HcodeInfo *phcInfo,int _hcode,char *_area,int i){
		if(isHcodeFull(phcInfo)){
			printf("Hcode is full\n");
			return -1;
		}
		phcInfo->phchead[phcInfo->cnt].hcode=_hcode;
		memset(phcInfo->phchead[phcInfo->cnt].areacode,'\0',sizeof(phcInfo->phchead[phcInfo->cnt].areacode));
		sprintf(phcInfo->phchead[phcInfo->cnt].areacode,"%s",_area);
		phcInfo->cnt=i;
		return 1;
	
	}


void printHcode(HcodeInfo *phcInfo)
{
	
	int i;
	/*if(isHcodeEmpty(&ptInfo)){
		printf("Trunks is empty\n");
	}else
	{
		for(i=0;i<ptInfo->cnt;i++){
			printf("local=%s\n %d telecomid=%d,trunkid=%d\n",ptInfo->trunkLocal,ptInfo->phead[i].telecomId,ptInfo->phead[i].trunkId);
		}
	}*/
	
	for(i=0;i<phcInfo->len;i++){
			printf("No %d is hcode=%d,areacode=%s\n",i,phcInfo->phchead[i].hcode,phcInfo->phchead[i].areacode);
		}

}

int getAreacode(HcodeInfo *phcInfo,int _hcode,int from,int to,char* _areacode)
{
	int low=from;
	int high=to;
	int mid=0;
	
	memset(_areacode,'\0',sizeof(_areacode));
	
	while(low<=high){
		mid=(low+high)/2;
		if(phcInfo->phchead[mid].hcode==_hcode){
			 sprintf(_areacode,"%s",phcInfo->phchead[mid].areacode);
			 return 1;
			}
		if(phcInfo->phchead[mid].hcode<_hcode) 
			{low=mid+1;
			}else
			{
					high=mid-1;
			}
	}
	 sprintf(_areacode,"%s","0000");
	return -1;
	
}


void initHcodeindexInfo(HcodeindexInfo *phciInfo,int length){
		phciInfo->phcihead=(hcodeindex *)malloc(sizeof(hcodeindex)*length);
		if(NULL==phciInfo->phcihead)
		{
			printf("initHcodeindexInfo memeory out!\n");
			exit(-1);
		}else 
		{
				phciInfo->len=length;
				phciInfo->cnt=0;
		}
	
	}

int isHcodeindexEmpty(HcodeindexInfo *phciInfo)
{
	if(0==phciInfo->cnt){
		return 1;
	}else{
		return 0;
	}
}
	
int isHcodeindexFull(HcodeindexInfo *phciInfo){
	if (phciInfo->len==phciInfo->cnt){
		return 1;							//满了返回1
	}else
	{
			return 0;						//不满返回0
	}
	
}

int appendHcodeindex(HcodeindexInfo *phciInfo,int seg,int _from,int _to,int i){
		if(isHcodeindexFull(phciInfo)){
			printf("Hcodeindex is full\n");
			return -1;
		}
		phciInfo->phcihead[phciInfo->cnt].hcodeseg=seg;
		phciInfo->phcihead[phciInfo->cnt].from=_from;
		phciInfo->phcihead[phciInfo->cnt].to=_to;
		phciInfo->cnt=i;
		return 1;
	
	}


void printHcodeindex(HcodeindexInfo *phciInfo)
{
	
	int i;
	/*if(isHcodeEmpty(&ptInfo)){
		printf("Trunks is empty\n");
	}else
	{
		for(i=0;i<ptInfo->cnt;i++){
			printf("local=%s\n %d telecomid=%d,trunkid=%d\n",ptInfo->trunkLocal,ptInfo->phead[i].telecomId,ptInfo->phead[i].trunkId);
		}
	}*/
	
	for(i=0;i<phciInfo->len;i++){
			printf("No %d is hcodeseq=%d,from=%d,to=%d\n",i,phciInfo->phcihead[i].hcodeseg,phciInfo->phcihead[i].from,phciInfo->phcihead[i].to);
		}

}

int getHcodeindex(HcodeindexInfo *phciInfo,int seg)
{
	int low=0;
	int high=phciInfo->len-1;
	int mid=0;
	
	
	while(low<=high){
		mid=(low+high)/2;
		if(phciInfo->phcihead[mid].hcodeseg==seg){
			 
			 return mid;
			}
		if(phciInfo->phcihead[mid].hcodeseg<seg) 
			{low=mid+1;
			}else
			{
					high=mid-1;
			}
	}

	return -1;
	
}
int readHcodeindex(HcodeindexInfo phciInfo){
	
	FILE *fp=NULL;
	char filename[32];
	int readflag=0;
	
	char buf[64];
	char s[64];
	
	char *delim;
	char *p;
	char ch;
	char temp[64];
	int i=1;
	int hcodeseg=0;
	int from =0;
	int to=0;

	

	memset(filename,'\0',sizeof(filename));
	sprintf(filename,"%s","./hcodecfg.ini");
	fp=fopen(filename,"r");
	if (NULL==fp){
		printf("open file %s is failture\n",filename);	
		return -1;
	}
	

	
	 while (!feof(fp))
	 {
	 	
	 		if(fgets(buf,sizeof(buf),fp)!=NULL){
	 				strcpy(s,trim(buf));
	 				
	 				ch=buf[0];
	 				if(ch=='#' ||ch==' '||  ch=='\n'||ch=='\0' ||ch==';')
	 					continue;
	 				
	 				if (ch=='('){
	 						delim=",";
	 						readflag=1;
	 						continue;
	 				}
	 				
	 				
	 				if (ch==')'){
	 					readflag=0;
	 					break;
	 				}
	 				
	 				p=strtok(s,delim);
	 				
	 				//printf("first is %s ",p);
	 				
	 				if (readflag==1){
	 					hcodeseg=atoi(p);
          	//len=strlen(s);
          	memset(temp,0,sizeof(temp));
         	 	sprintf(temp,s+strlen(p)+1);
          	memset(s,0,sizeof(s));
          	sprintf(s,temp);
          	p=strtok(s,delim);
          	from=atoi(p);
          	memset(temp,0,sizeof(temp));
         	 	sprintf(temp,s+strlen(p)+1);
          	memset(s,0,sizeof(s));
          	sprintf(s,temp);
          	to=atoi(s);
           	//printf("scend is %s\n",s);
               // p=strtok(s,delim);
            appendHcodeindex(&phciInfo,hcodeseg,from,to,i);
            i++;
	 				}	
	 				//}
	 					 					
	 		}	
	 	} 
	 	return 1;	
}





int getLen(char* cfgfile){
	FILE *fp=NULL;
	char filename[32];	
	char buf[64];
	char s[64];
	char temp[64];
	char ch;
	int len;
	char *delim;
	char *p;
	memset(filename,'\0',sizeof(filename));
	sprintf(filename,cfgfile);
	int le=0;
	
	fp=fopen(filename,"r");
	if(fp==NULL){
		printf ("open file %s erro\n",filename);
		return 0;
	}
	
	while(!feof(fp)){
		if(fgets(buf,sizeof(buf),fp)!=NULL){
	 				strcpy(s,trim(buf));
	 				
	 				//printf("read is %s\n",s);
	 				ch=buf[0];
	 				if(ch=='#' ||ch==' '||  ch=='\n'||ch=='\0')
	 					continue;
	 				
	 				if (ch==';'){
	 						 	delim="=";
	 						 	p=strtok(s,delim);
	 						 	
	 						 	
	 						 	if(strncmp(";length",p,7)==0){
	 						 		len=strlen(s);
          				memset(temp,0,sizeof(temp));
         	 				sprintf(temp,s+strlen(p)+1);
          				memset(s,0,sizeof(s));
          				sprintf(s,trim(temp));
           				//printf("scend is %s",s);
           				le=atoi(s);
           				//printf("length=%d\n",length);
           				fclose(fp);
           				return le;
           				
	 						 	}else{
	 						 		continue;
	 						 	}		
           			
	 				}	else {
	 						continue;
	 				}
	 				
		}
		fclose(fp);
		break;
		return le;
		
	}
	return le;	
}


int readHcode(HcodeInfo phcInfo){
	
	FILE *fp=NULL;
	char filename[32];
	int readflag=0;
	
	char buf[64];
	char s[64];
	
	char *delim;
	char *p;
	char ch;
	int len;
	char temp[64];
	int i=1;
	int hcode=0;

	

	memset(filename,'\0',sizeof(filename));
	sprintf(filename,"%s","./hcode.ini");
	fp=fopen(filename,"r");
	if (NULL==fp){
		printf("open file %s is failture\n",filename);	
		return -1;
	}
	

	
	 while (!feof(fp))
	 {
	 	
	 		if(fgets(buf,sizeof(buf),fp)!=NULL){
	 				strcpy(s,trim(buf));
	 				
	 				ch=buf[0];
	 				if(ch=='#' ||ch==' '||  ch=='\n'||ch=='\0' ||ch==';')
	 					continue;
	 				
	 				if (ch=='('){
	 						delim=",";
	 						readflag=1;
	 						continue;
	 				}
	 				
	 				
	 				if (ch==')'){
	 					readflag=0;
	 					break;
	 				}
	 				
	 				p=strtok(s,delim);
	 				
	 				//printf("first is %s ",p);
	 				
	 				if (readflag==1){
	 					hcode=atoi(p);
	 					if (hcode>10000000){					//8位hcode
	 						hcode=hcode/10;
	 					}
          	len=strlen(s);
          	memset(temp,0,sizeof(temp));
         	 	sprintf(temp,s+strlen(p)+1);
          	memset(s,0,sizeof(s));
          	sprintf(s,temp);
           	//printf("scend is %s\n",s);
               // p=strtok(s,delim);
            appendHcode(&phcInfo,hcode,s,i);
            i++;
	 				}	
	 				//}
	 					 					
	 		}	
	 	} 
	 		return 1;	
	}


void initCenNumInfo(CenNumInfo *pcniInfo,int length){
		pcniInfo->pcnhead=(cenNum *)malloc(sizeof(cenNum)*length);
		if(NULL==pcniInfo->pcnhead)
		{
			printf("initCenNumInfo memeory out!\n");
			exit(-1);
		}else 
		{
				pcniInfo->len=length;
				pcniInfo->cnt=0;
		}
	
	}

int isCenNumEmpty(CenNumInfo *pcniInfo)
{
	if(0==pcniInfo->cnt){
		return 1;
	}else{
		return 0;
	}
}
	
int isCenNumFull(CenNumInfo *pcniInfo){
	if (pcniInfo->len==pcniInfo->cnt){
		return 1;							//满了返回1
	}else
	{
			return 0;						//不满返回0
	}
	
}

int appendCenNum(CenNumInfo *pcniInfo,int _centerNum,char *_area,int i){
		if(isCenNumFull(pcniInfo)){
			printf("CenNum is full\n");
			return -1;
		}
		pcniInfo->pcnhead[pcniInfo->cnt].centerNum=_centerNum;
		memset(pcniInfo->pcnhead[pcniInfo->cnt].areacode,'\0',sizeof(pcniInfo->pcnhead[pcniInfo->cnt].areacode));
		sprintf(pcniInfo->pcnhead[pcniInfo->cnt].areacode,"%s",_area);
		pcniInfo->cnt=i;
		return 1;
	
	}


void printCenNum(CenNumInfo *pcniInfo)
{
	
	int i;
	/*if(isHcodeEmpty(&ptInfo)){
		printf("Trunks is empty\n");
	}else
	{
		for(i=0;i<ptInfo->cnt;i++){
			printf("local=%s\n %d telecomid=%d,trunkid=%d\n",ptInfo->trunkLocal,ptInfo->phead[i].telecomId,ptInfo->phead[i].trunkId);
		}
	}*/
	
	for(i=0;i<pcniInfo->len;i++){
			printf("No %d is centernum=%d,areacode=%s\n",i,pcniInfo->pcnhead[i].centerNum,pcniInfo->pcnhead[i].areacode);
		}
	return;
}

int getCenNumcode(CenNumInfo *pcniInfo,int _centerNum,char* _areacode)
{
	int low=0;
	int high=pcniInfo->len;
	int mid=0;
	
	memset(_areacode,'\0',sizeof(_areacode));
	
	while(low<=high){
		mid=(low+high)/2;
		if(pcniInfo->pcnhead[mid].centerNum==_centerNum){
			 sprintf(_areacode,"%s",pcniInfo->pcnhead[mid].areacode);
			 return 1;
			}
		if(pcniInfo->pcnhead[mid].centerNum<_centerNum) 
			{low=mid+1;
			}else
			{
					high=mid-1;
			}
	}
	 sprintf(_areacode,"%s","0000");
	return -1;
	
}


int readCenNum(CenNumInfo pcniInfo){
	
	FILE *fp=NULL;
	char filename[32];
	int readflag=0;
	
	char buf[64];
	char s[64];
	
	char *delim;
	char *p;
	char ch;
	int len;
	char temp[64];
	int i=1;
	int centerNum=0;


	memset(filename,'\0',sizeof(filename));
	sprintf(filename,"%s","./centerNum.ini");
	fp=fopen(filename,"r");
	if (NULL==fp){
		printf("open file %s is failture\n",filename);	
		return -1;
	}
	

	
	 while (!feof(fp))
	 {
	 	
	 		if(fgets(buf,sizeof(buf),fp)!=NULL){
	 				strcpy(s,trim(buf));
	 				
	 				ch=buf[0];
	 				if(ch=='#' ||ch==' '||  ch=='\n'||ch=='\0' ||ch==';')
	 					continue;
	 				
	 				if (ch=='('){
	 						delim=",";
	 						readflag=1;
	 						continue;
	 				}
	 				
	 				
	 				if (ch==')'){
	 					readflag=0;
	 					break;
	 				}
	 				
	 				p=strtok(s,delim);
	 				
	 				//printf("first is %s ",p);
	 				
	 				if (readflag==1){
	 					centerNum=atoi(p);
	 					
          	len=strlen(s);
          	memset(temp,0,sizeof(temp));
         	 	sprintf(temp,s+strlen(p)+1);
          	memset(s,0,sizeof(s));
          	sprintf(s,temp);
           	//printf("scend is %s\n",s);
               // p=strtok(s,delim);
            appendCenNum(&pcniInfo,centerNum,s,i);
            i++;
	 				}	
	 				//}
	 					 					
	 		}	
	 	} 
	 		return 1;	
	}
	

int MakeLine(pLatn ticket,char *strline)
{
	sprintf(strline,"%d,%d,%d,%d,%d,%d,%s,%s,%s,%04d/%02d/%02d %s,%04d/%02d/%02d %s,%d,%d,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s,%s",
		ticket->csn,
		ticket->Partialflag,ticket->validflag,ticket->freeflag,ticket->chargeparty,ticket->termination,
		ticket->callingphone,ticket->calledphone,ticket->dialnumber,
		ticket->byear,ticket->bmonth,ticket->bday,
		ticket->begintime,
		ticket->eyear,ticket->emonth,ticket->eday,
		ticket->endtime,
		ticket->duration,ticket->feedur,ticket->timelong,ticket->fee,
		ticket->callingareacode,ticket->calledareacode,ticket->centerareacode,ticket->intrunks,ticket->outtrunks,ticket->inTeleId,ticket->outTeleId,ticket->calltype,
		ticket->innerCalltype,ticket->custId,ticket->eday,ticket->roamflag,
		ticket->sourcefilename,ticket->errno,ticket->billingcycleid);
	return 1;	
}

int GetHwFormatInfo(strSwc)
	Hwinfo * strSwc;
{
	strcpy(strSwc->flag,"3000");
	/*取话单总长度*/
	strSwc->lenpack=907;
	/*取话单序号*/
	strSwc->sposcsn=0;
	strSwc->lencsn=4;
	
	/*取话单类型*/
	strSwc->sposbilltype=7;
	strSwc->lenbilltype=1;
	
	/*取话单标志位*/
	strSwc->sposPFflag=9;
	strSwc->lenPFflag=1;
	
	/*取主叫号码*/
	strSwc->sposcalling=30;
	strSwc->lencalling=16;
	
	/*取被叫号码*/
	strSwc->sposcalled=49;
	strSwc->lencalled=16;
	
	/*取拨叫号码被叫*/
	strSwc->sposdn=139;
	strSwc->lendn=16;
	
	/*取开始日期*/
	strSwc->sposbdate=11;
	strSwc->lenbdate=6;
	
	/*取结束日期开始位置*/
	strSwc->sposedate=17;
	strSwc->lenedate=6;
	
	/*取时长开始位置*/
	strSwc->sposdur=23;
	strSwc->lendur=4;
	
	/*取入中继开始位置*/
	strSwc->sposintrunks=77;
	strSwc->lenintrunks=2;
	
	/*取出中继开始位置*/
	strSwc->sposouttrunks=79;
	strSwc->lenouttrunks=2;
	
	strSwc->sposchargeparty=10;
	strSwc->lenchargeparty=1;
	
	strSwc->spostermination=87;
	strSwc->lentermination=1;

  return 1;
}


//获得处理下标
/*
int GetHwSuffix(HWTotal hwtotal)
{
  int kkk;
  for(kkk=0;kkk<hwtotal.count;kkk++)	
  {
    if (strcmp(hwtotal.hwinfo[kkk].flag,"08")==0)  return kkk;
    return -1;
        
  }
}
*/

int GetHwCsn(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int i=begin;
	ticket->csn=0;
  ticket->csn=buf[i]+buf[i+1]*256+buf[i+2]*256*256+buf[i+3]*256*256*256;
  return 1;
}

int GetHwFlag(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int i=begin;
	ticket->Partialflag=buf[i]&15;
	ticket->validflag=(buf[i]&16)>>4;
	ticket->freeflag=(buf[i]&32)>>5;
	return 1;
}
//----------------
int GetChargeparty(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int i=begin;  //10
	ticket->chargeparty=(buf[i]&0xF0)>>4;
	return 1;
}

int GetTermination(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int i=begin;  //87
	ticket->termination=buf[i];
	return 1;
}
//--------------add by liudq@20171014

int GetHwNbr(unsigned char * buf, pLatn ticket,int begin,int len,int flag)
{
  int kkk,j=0;
  char ss[3],cAccNbr[20];
  int end;
  end=begin+len-1;
  memset(cAccNbr,'\0',sizeof(cAccNbr));
  for (kkk=begin;kkk<=end;kkk++)
  {
    if ((buf[kkk]&240)==240) 
    {
    	 cAccNbr[j]='\0';
    	 break;
    }
    
    sprintf(ss,"%02x",buf[kkk]);
    ss[2]='\0';
    strcat(cAccNbr,ss);
    j+=2;
    
    if ((buf[kkk]&15)==15)
    {
     cAccNbr[j-1]='\0';
    	 break;
    }
  }
  switch(flag){
  	case 0:
  		strcpy(ticket->callingphone,cAccNbr);
  		break;
  	case 1:
  		strcpy(ticket->calledphone,cAccNbr);
  		break;
  	case 2:
  		strcpy(ticket->dialnumber,cAccNbr);
  		break;
  	default:
  		break;
  }
  return 1;
}



int GetHwbTime(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int hour,min,sec;
	int i=begin;
	ticket->byear=0;
	ticket->byear = buf[i++] + 2000;
	ticket->bmonth=0;
  ticket->bmonth = buf[i++];
  ticket->bday=0;
  ticket->bday = buf[i++];
  hour=buf[i++];
  min=buf[i++];
  sec=buf[i++];
  memset(ticket->begintime,'\0',sizeof(ticket->begintime));
  sprintf(ticket->begintime,"%02d:%02d:%02d",hour,min,sec);
  //printf ("GetHwbTime[ticket->byear=%04d]\n",ticket->byear);
 	//printf("[ticket->begintime=%s]\n",strTicket->begintime);
 	return 1;
}

int GetHweTime(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int hour, min, sec;
	int i=begin;
	ticket->eyear = buf[i++] + 2000;
  ticket->emonth = buf[i++];
  ticket->eday = buf[i++];
  hour=buf[i++];
  min=buf[i++];
  sec=buf[i++];
  memset(ticket->endtime,'\0',sizeof(ticket->endtime));
  sprintf(ticket->endtime,"%02d:%02d:%02d",hour,min,sec);
  sprintf(ticket->billingcycleid,"%04d%02d%02d",ticket->eyear,ticket->emonth,ticket->eday);
 	//printf("[ticket->begintime=%s]\n",strTicket->begintime);
 	return 1;
}

int GetHwDur(unsigned char * buf, pLatn ticket,int begin,int len)
{
	int i=begin;
	ticket->duration=0;
  ticket->duration=buf[i]+buf[i+1]*256+buf[i+2]*256*256+buf[i+3]*256*256*256;
  return 1;
}

int GetHwTrunks(unsigned char * buf, pLatn ticket,int begin,int len,int flag)
{
	int i=begin;
	if(flag==0){
	  	ticket->intrunks=0;
  	ticket->intrunks=buf[i]+buf[i+1]*256;
	}else {
	  	ticket->outtrunks=0;
  	ticket->outtrunks=buf[i]+buf[i+1]*256;
  }
  return 1;
}




int GetTeleid(pLatn ticket,ptrunkInfo ptInfo){
	ticket->inTeleId=getTelecomId(ptInfo,ticket->intrunks);
	ticket->outTeleId=getTelecomId(ptInfo,ticket->outtrunks);
	return 1;
	
}

/*inteleid<10 and inteleid<>0 and outteleid>10 0*/
/*inteleid>10 and outteleid<>0 and outteleid<10 1*/
/*inteleid<10 and inteleid<>0 and (outtrunks=4 or outtrunks=2005 2*/
/*inteleid>10 and outteleid>10 3*/
/*(intrunks=4 or intrunks=2005) and outteleid>10 4 */
             
int GetCalltype(pLatn ticket){
	if(ticket->inTeleId<10 && ticket->inTeleId!=0 && ticket->outTeleId>=10)
	{
			ticket->calltype=0;
			return 0;
	}
	
	if(ticket->inTeleId>=10 && ticket->outTeleId!=0 && ticket->outTeleId<10)
	{
			ticket->calltype=1;
			return 0;
	}
	
	if(ticket->inTeleId!=0 && (ticket->outtrunks==4||ticket->outtrunks==2005))
	{
			ticket->calltype=2;
			return 0;
	}

/*	if(ticket->outTeleId!=0 && ticket0>outTeleId<10 && (ticket->intrunks==4||ticket->intrunks==2005))
	{
			ticket->calltype=5;
			return 0;
	} */
	
	if(ticket->inTeleId>=10 && ticket->outTeleId>=10)
	{
			ticket->calltype=3;
			return 0;
	}
	
	if( (ticket->intrunks==4||ticket->intrunks==2005)&& ticket->outTeleId>=10)
	{
			ticket->calltype=4;
			return 0;
	}
	ticket->calltype=-1;
	return -1;
}

int GetICTC(pLatn ticket){
	char buf[8];	
	char callingbuf[8];	
	int numhead;
	
	
	memset(buf,'\0',sizeof(buf));
	if (ticket->calltype==0){
		ticket->innerCalltype=0;
		strncpy(buf,ticket->calledphone,5);
		numhead=atoi(buf);
		if(numhead==95338 || numhead==55684 || numhead==84466){
			ticket->custId=1;
			return 0;
		}
		
		if(numhead==95183 ){
			ticket->custId=2;
			return 0;
		}
		memset(buf,'\0',sizeof(buf));
		strncpy(buf,ticket->calledphone,2);
		numhead=atoi(buf);
		if(numhead==11 || numhead==12){
			ticket->custId=1;
			return 0;
		}
		//add by liudq@20151125
		if(strncmp(ticket->dialnumber,"95338",5)==0 ){
			ticket->custId=1;
			return 0;
		}
		
		ticket->custId=0;
		return 0;
	}
	if (ticket->calltype==1){
		ticket->innerCalltype=0;
		strncpy(buf,ticket->callingphone,3);
		numhead=atoi(buf);
		if(numhead==244){
			ticket->custId=1;
			return 0;
		}
		memset(buf,'\0',sizeof(buf));
		strncpy(buf,ticket->callingphone,2);
		numhead=atoi(buf);
		if(numhead==11 || numhead==12){
			ticket->custId=1;
			return 0;
		}
		if(strncmp(ticket->callingphone,"83876666",8)==0 ||strncmp(ticket->callingphone,"4008111111",10)==0 ){
			ticket->custId=1;
			return 0;
		}
		
		//add by liudq@20160927
		memset(buf,'\0',sizeof(buf));
		strncpy(buf,ticket->callingphone,5);
		numhead=atoi(buf);
		if(numhead==95183){
			ticket->custId=2;
			return 0;
		}
		
		if(numhead==95338){
			ticket->custId=1;
			return 0;
		}
		
		ticket->custId=0;
		return -1;
	}
	
	if((ticket->calltype!=0)&&(ticket->calltype!=1)){
		strncpy(buf,ticket->calledphone,5);
		numhead=atoi(buf);
		if(numhead==95338 || numhead==55684 || numhead==84466){
			ticket->custId=1;
			ticket->innerCalltype=2;
			return 0;
		}
		
		if(numhead==95183){
			ticket->custId=2;
			return 0;
		}
		
		memset(buf,'\0',sizeof(buf));
		strncpy(buf,ticket->calledphone,3);
		numhead=atoi(buf);
		if(numhead==244){
			ticket->custId=1;
			memset(callingbuf,'\0',sizeof(callingbuf));
			if(strncmp(ticket->callingphone,"244",3)==0 ||(strncmp(ticket->callingphone,"0244",4)==0 && strlen(ticket->callingphone)<11)){
				ticket->innerCalltype=1;
				return 0;
			}
			ticket->innerCalltype=2;
			return 0;
		}
		
		memset(buf,'\0',sizeof(buf));
		strncpy(buf,ticket->calledphone,2);
		numhead=atoi(buf);
		if(numhead==11 || numhead==12){
			ticket->custId=1;
			ticket->innerCalltype=2;
			return 0;
		}
		
		if(strncmp(ticket->dialnumber,"95338",5)==0 ){
			ticket->custId=1;
			return 0;
		}
		
		
		ticket->custId=0;
		ticket->innerCalltype=-1;
		strcat(ticket->errno,"1");
		return 0;
	}
	
	
	
	return -1;
}	


int GetTimeFee(pLatn ticket){
	//ticket->feedur=(ticket->duration+50)/100;
	ticket->feedur=(ticket->duration+99)/100;			//向上取整 ceil() modify by liudq@20151130
	if(ticket->feedur==0 && ticket->duration!=0)   //不足半秒的处理成1秒
		{
			ticket->feedur=1;
		}
	ticket->timelong=ceil((float)ticket->feedur/(float)60);
	
	if(ticket->calltype==0){
		ticket->fee=ticket->timelong*45;
		return 0;
	}
	
	if(ticket->calltype==1){
		ticket->fee=ticket->timelong*60;
		return 0;
	}
	
	if ((ticket->calltype==2)&&(ticket->innerCalltype==1)){
		ticket->fee=ticket->timelong*60;
		return 0;
	}
	
	if ((ticket->calltype==2)&&(ticket->innerCalltype==2)){
		ticket->fee=0;
		return 0;
	}
	
	if ((ticket->calltype==3)&&(ticket->innerCalltype==2)){
		ticket->fee=0;
		return 0;
	}
	
	if ((ticket->calltype==3)&&(ticket->innerCalltype==1)){
		ticket->fee=ticket->timelong*45;
		return 0;
	}
	
	if ((ticket->calltype==4)&&(ticket->innerCalltype==2)){
		ticket->fee=0;
		return 0;
	}

	if ((ticket->calltype==4)&&(ticket->innerCalltype==1)){
		ticket->fee=ticket->timelong*60;
		return 0;
	}
	return -1;
}



int GetHwAreacode(pLatn ticket,phcodeInfo phcInfo,phcodeindexInfo phciInfo)   //待编写
{
	int phonetype;
	char nhead[4];
	char seg[8];
	char number[16];
	char area[8];
	char area2[8];
	int index;
	
	memset(number,'\0',sizeof(number));	
	memset(area,'\0',sizeof(area));	
	memset(area2,'\0',sizeof(area2));	
	memset(nhead,'\0',sizeof(nhead));	
	memset(seg,'\0',sizeof(seg));	
	
	if(ticket->calltype==0 ||ticket->innerCalltype ==2){							//呼入话单处理主叫
		sprintf(number,"%s",ticket->callingphone);
		phonetype=getPhonetype(number);
		if(phonetype==5){
			
			if(strncmp(ticket->callingphone,"244",3)==0){
					strncpy(area,ticket->callingphone+3,3);
					area[3]='\0';
					if(area[0]=='0' && strlen(ticket->callingphone)>=8){
							sprintf(ticket->callingareacode,"%s",area);
					}else{
							sprintf(ticket->callingareacode,"0%s",area);
					}
					sprintf(ticket->calledareacode,"%s","0000");
					return 1;
				}
				
				if(strncmp(ticket->callingphone,"0244",4)==0 && strlen(ticket->callingphone) < 11){
					strncpy(area,ticket->callingphone+4,3);
					area[3]='\0';
					if(area[0]=='0' && strlen(ticket->callingphone)>=8){
							sprintf(ticket->callingareacode,"%s",area);
					}else{
							sprintf(ticket->callingareacode,"0%s",area);
					}
					sprintf(ticket->calledareacode,"%s","0000");
					return 1;
				}
			sprintf(ticket->callingareacode,"%s","0000");
			sprintf(ticket->calledareacode,"%s","0000");
			strcat(ticket->errno,"2");
			return 1;			
		}
		if(phonetype==1){
			//strncpy(area,number,4);
			if(strncmp(number,"01",2)==0 ||strncmp(number,"02",2)==0){
				strncpy(area,number,3);
				sprintf(ticket->callingareacode,"%s",area);
				sprintf(ticket->calledareacode,"%s","0000");
				return 1;			
			}
			strncpy(area,number,4);
			sprintf(ticket->callingareacode,"%s",area);
			sprintf(ticket->calledareacode,"%s","0000");
			return 1;			
		}
		if(phonetype==2){
			strcpy(number,deleZero(number));
			strncpy(nhead,number,3);
			strncpy(seg,number,7);
			index=getHcodeindex(phciInfo,atoi(nhead));
			getAreacode(phcInfo,atoi(seg),phciInfo->phcihead[index].from,phciInfo->phcihead[index].to,area);
			sprintf(ticket->callingareacode,"%s",area);
			sprintf(ticket->calledareacode,"%s","0000");
			return 1;
			
		}
		sprintf(ticket->callingareacode,"%s","0000");
		sprintf(ticket->calledareacode,"%s","0000");
		strcat(ticket->errno,"2");
		return 1;
	}
	
	if(ticket->calltype==1 ||ticket->innerCalltype ==1){							//呼出话单处理主叫
		
		
		sprintf(number,"%s",ticket->calledphone);
			//去掉701
		if(strncmp(number,"701",3)==0 && strlen(number) >= 10){
			
			strcpy(number,delePrex(number,3));
		
		}
		
//		if(strncmp(number,"901",3)==0 && strlen(number) >= 10){ --modify ldq@20170202
		if(strncmp(number,"9",1)==0 && strncmp(number,"95338",5)!=0 && strncmp(number,"95183",5)!=0 && strlen(number) >= 10){	
			strcpy(number,delePrex(number,3));
		
		}
		
		phonetype=getPhonetype(number);
		sprintf(ticket->callingareacode,"%s","0000");
		
		if((strncmp(ticket->callingphone,"0244",4)==0) && strlen(ticket->callingphone)<11){
					strncpy(area2,ticket->callingphone+4,3);
					area2[3]='\0';
					if(area2[0]=='0' && strlen(ticket->callingphone)>=8){
							sprintf(ticket->callingareacode,"%s",area2);
					}else{
							sprintf(ticket->callingareacode,"0%s",area2);
					}
					//sprintf(ticket->calledareacode,"%s","0000");
				}
		
		if(strncmp(ticket->callingphone,"244",3)==0){
			strncpy(area2,ticket->callingphone+3,3);
			area2[3]='\0';
			if(area2[0]=='0' && strlen(ticket->callingphone)>=8){
					sprintf(ticket->callingareacode,"%s",area2);
			}else{
					sprintf(ticket->callingareacode,"0%s",area2);
			}
		
		}

	if((strncmp(ticket->callingphone,"11",2)==0)||(strncmp(ticket->callingphone,"12",2)==0)){
			strncpy(area2,ticket->callingphone+2,3);
			area2[3]='\0';
			if(area2[0]=='0' && strlen(ticket->callingphone)>=5){
					sprintf(ticket->callingareacode,"%s",area2);
			}else{
					sprintf(ticket->callingareacode,"0%s",area2);
			}
		
		}
		
		
		if(phonetype==5){
			//sprintf(ticket->callingareacode,"%s","0000");
			
			if(strstr(ticket->dialnumber,"95338")!=NULL && strncmp(ticket->dialnumber,"95338",5)!=0){
				strncpy(area,ticket->dialnumber,strlen(ticket->dialnumber)-strlen(strstr(ticket->dialnumber,"95338")));
				area[strlen(ticket->dialnumber)-strlen(strstr(ticket->dialnumber,"95338"))]='\0';
				sprintf(ticket->calledareacode,"%s",area);
				return 1;
			}
			
			
			sprintf(ticket->calledareacode,"%s","0000");
			strcat(ticket->errno,"2");
			return 1;			
		}
		if(phonetype==1){
			//strncpy(area,number,4);
			if(strncmp(number,"01",2)==0 ||strncmp(number,"02",2)==0){
				strncpy(area,number,3);
				area[3]='\0';
				//sprintf(ticket->callingareacode,"%s","0000");
				sprintf(ticket->calledareacode,"%s",area);
				return 1;			
			}
			strncpy(area,number,4);
			area[4]='\0';
			//sprintf(ticket->callingareacode,"%s","0000");
			sprintf(ticket->calledareacode,"%s",area);
			return 1;			
		}
		if(phonetype==2){
			strcpy(number,deleZero(number));
			strncpy(nhead,number,3);
			strncpy(seg,number,7);
			index=getHcodeindex(phciInfo,atoi(nhead));
			getAreacode(phcInfo,atoi(seg),phciInfo->phcihead[index].from,phciInfo->phcihead[index].to,area);
			//sprintf(ticket->callingareacode,"%s","0000");
			sprintf(ticket->calledareacode,"%s",area);
			return 1;
			
		}
				
		sprintf(ticket->callingareacode,"%s","0000");
		sprintf(ticket->calledareacode,"%s","0000");
		strcat(ticket->errno,"2");
		return 1;
	}
	
	sprintf(ticket->callingareacode,"%s","0000");
	sprintf(ticket->calledareacode,"%s","0000");
	strcat(ticket->errno,"2");
	return 1;
}	
	

int GetCenterAreaCode(pLatn ticket,pcennumInfo pcniInfo) {
	char area[8];
	char tmp[16];
	char *t;
	memset(area,'\0',sizeof(area));
	memset(tmp,'\0',sizeof(tmp));
	

	
	if(ticket->calltype==0 || ticket->calltype==4 || ticket->calltype==2){
			sprintf(ticket->calledareacode,"%s","0000");
		
		if((strncmp(ticket->dialnumber,"11",2)==0)||(strncmp(ticket->dialnumber,"12",2)==0)){
				strncpy(area,ticket->dialnumber+2,3);
				area[3]='\0';
				if(area[0]=='0' && strlen(ticket->dialnumber)>=5){
						sprintf(ticket->calledareacode,"%s",area);
				}else{
						sprintf(ticket->calledareacode,"0%s",area);
				}
				return 1;
			}
		
		if(strncmp(ticket->dialnumber,"95338",5)==0 && strlen(ticket->dialnumber)==5 && ticket->inTeleId==1){
			sprintf(area,"%s",ticket->callingareacode);
			sprintf(ticket->calledareacode,"%s",area);
			return 1;
		}
		
		if(strncmp(ticket->dialnumber,"95338",5)==0 && strlen(ticket->dialnumber)==8){
			strncpy(area,ticket->dialnumber+5,3);
			if(area[0]=='0' ){
					sprintf(ticket->calledareacode,"%s",area);
			}else{
					sprintf(ticket->calledareacode,"0%s",area);
			}
		return 1;
		}

		if(strncmp(ticket->dialnumber,"95594",5)==0 && strlen(ticket->dialnumber)==8){
			strncpy(area,ticket->dialnumber+5,3);
			if(area[0]=='0' ){
					sprintf(ticket->calledareacode,"%s",area);
			}else{
					sprintf(ticket->calledareacode,"0%s",area);
			}
		return 1;
		}

		
		if(strncmp(ticket->dialnumber,"95338",5)==0 && strlen(ticket->dialnumber)>8){    
			strncpy(tmp,ticket->dialnumber+5,4);
			if((tmp[0]=='0')&&(tmp[1]=='1' || tmp[1]=='2' ) ){
					strncpy(area,tmp,3);
					area[3]='\0';
					sprintf(ticket->calledareacode,"%s",area);
					return 1;
			}
			
			if(tmp[0]=='0' ){
				strncpy(area,tmp,4);
				area[4]='\0';
				sprintf(ticket->calledareacode,"%s",area);
				return 1;
			}
			
			if (tmp[0]=='1' || tmp[0]=='2'){
				strncpy(area,tmp,2);
				area[2]='\0';
				sprintf(ticket->calledareacode,"0%s",area);
				return 1;
			}
		
				strncpy(area,tmp,3);
				area[3]='\0';	
				sprintf(ticket->calledareacode,"0%s",area);
				return 1;
		}
		
		t=strstr(ticket->dialnumber,"95338");  //0531+95338
		if(t!=NULL && strncmp(ticket->dialnumber,"95338",5)!=0){
			if (strlen(ticket->dialnumber)-strlen(t)>=8)      //modify by liudq@20180501
				{
					sprintf(ticket->calledareacode,"%s","0000");
					return 1;
				}
			strncpy(area,ticket->dialnumber,strlen(ticket->dialnumber)-strlen(t));
			area[strlen(ticket->dialnumber)-strlen(t)]='\0';
			sprintf(ticket->calledareacode,"%s",area);
			return 1;
		}
		
		if(strncmp(ticket->dialnumber,"55684",5)==0 || strncmp(ticket->dialnumber,"84466",5)==0){
			getCenNumcode(pcniInfo,atoi(ticket->dialnumber),area);
			sprintf(ticket->calledareacode,"%s",area);
			return 1;
		}
		
		if(strncmp(ticket->calledphone,"244",3)==0){
				strncpy(area,ticket->calledphone+3,3);
				area[3]='\0';
				if(area[0]=='0' && strlen(ticket->calledphone)>=8){
						sprintf(ticket->calledareacode,"%s",area);
				}else{
						sprintf(ticket->calledareacode,"0%s",area);
				}
				return 1;
			}
		
	}
	
	if(ticket->calltype==3){
			sprintf(ticket->calledareacode,"%s","0000");
			if(strncmp(ticket->calledphone,"244",3)==0){
				strncpy(area,ticket->calledphone+3,3);
				area[3]='\0';
				if(area[0]=='0' && strlen(ticket->calledphone)>=8){
						sprintf(ticket->calledareacode,"%s",area);
				}else{
						sprintf(ticket->calledareacode,"0%s",area);
				}
				return 1;
			}

			if((strncmp(ticket->calledphone,"11",2)==0)||(strncmp(ticket->calledphone,"12",2)==0)){
				strncpy(area,ticket->calledphone+2,3);
				area[3]='\0';
				if(area[0]=='0' && strlen(ticket->calledphone)>=5){
						sprintf(ticket->calledareacode,"%s",area);
				}else{
						sprintf(ticket->calledareacode,"0%s",area);
				}
				return 1;
			}
		
			if(strncmp(ticket->calledphone,"95338",5)==0  && strlen(ticket->calledphone)==8){
				strncpy(area,ticket->calledphone+5,3);
				if(area[0]=='0'){
						sprintf(ticket->calledareacode,"%s",area);
				}else{
						sprintf(ticket->calledareacode,"0%s",area);
				}
				return 1;
			}
			
			
	}
	return -1;
	
} 

int GetRoamFlag(pLatn ticket){
	if(ticket->calltype==0){
		if(strncmp(ticket->callingareacode,ticket->calledareacode,strlen(ticket->calledareacode))==0){
			ticket->roamflag=0;
			return 1;
		}
		
		if(strncmp(ticket->callingareacode,"0000",4)==0){
			ticket->roamflag=0;
			if(strncmp(ticket->calledareacode,"01",2)==0 || strncmp(ticket->calledareacode,"02",2)==0){
					strncpy(ticket->callingareacode,ticket->calledareacode,3);
					ticket->callingareacode[3]='\0';
					return 1;
				}
				
			strncpy(ticket->callingareacode,ticket->calledareacode,strlen(ticket->calledareacode));
			return 1;
		}
			
		ticket->roamflag=1;
		return 1;
	
	}
	ticket->roamflag=2;
	return 1;
}

int GetJsAreacode(pLatn ticket){
	char area[8];
	memset(area,'\0',sizeof(area));

		sprintf(ticket->centerareacode,"%s","0000");
		if(ticket->calltype==0 || ticket->calltype==4 || ticket->calltype==2){
			sprintf(ticket->centerareacode,"%s","0000");
		
		if((strncmp(ticket->calledphone,"11",2)==0)||(strncmp(ticket->calledphone,"12",2)==0)){
				strncpy(area,ticket->calledphone+2,3);
				area[3]='\0';
				if(area[0]=='0' && strlen(ticket->calledphone)>=5){
						sprintf(ticket->centerareacode,"%s",area);
				}else{
						sprintf(ticket->centerareacode,"0%s",area);
				}
				return 1;
			}
			
			if(strncmp(ticket->calledphone,"2275501",7)==0){
				strncpy(area,"0755",4);
				area[4]='\0';
				sprintf(ticket->centerareacode,"%s",area);
				return 1;
			}
		}
		return 1;
}

int DealHwTicket(unsigned char * buf, pLatn ticket,Hwinfo hwinfo,char * filename,ptrunkInfo ptInfo,phcodeInfo phcInfo,phcodeindexInfo phciInfo,pcennumInfo pcniInfo)
{

  strcpy(ticket->errno,eNOERR);
  GetHwCsn(buf,ticket,hwinfo.sposcsn,hwinfo.lencsn);  //get csn
  GetHwFlag(buf,ticket,hwinfo.sposPFflag,hwinfo.lenPFflag);
  GetTermination(buf,ticket,hwinfo.spostermination,hwinfo.lentermination);    //get termiation
  GetHwNbr(buf,ticket,hwinfo.sposcalling,hwinfo.lencalling,0);	//get calling nbr
  GetHwNbr(buf,ticket,hwinfo.sposcalled,hwinfo.lencalled,1);	//get called nbr
  GetHwNbr(buf,ticket,hwinfo.sposdn,hwinfo.lendn,2);	//get dial nbr
  GetHwbTime(buf,ticket,hwinfo.sposbdate,hwinfo.lenbdate);
  GetHweTime(buf,ticket,hwinfo.sposedate,hwinfo.lenedate);
  GetHwDur(buf,ticket,hwinfo.sposdur,hwinfo.lendur);
  GetHwTrunks(buf,ticket,hwinfo.sposintrunks,hwinfo.lenintrunks,0);
  GetHwTrunks(buf,ticket,hwinfo.sposouttrunks,hwinfo.lenouttrunks,1);
	GetChargeparty(buf,ticket,hwinfo.sposchargeparty,hwinfo.lenchargeparty);  //get charge party

  strcpy(ticket->sourcefilename,filename);
  GetTeleid(ticket,ptInfo);
  GetCalltype(ticket);
  GetICTC(ticket);
  GetHwAreacode(ticket,phcInfo,phciInfo);
	GetCenterAreaCode(ticket,pcniInfo);
  GetTimeFee(ticket);
  GetRoamFlag(ticket);
  GetJsAreacode(ticket);
  return 1;
  
}

