#include "chshell.h"

/*==================================================================================
	CH SHELL 是一个灵活的串口组件，利用它可以用串口调用程序中任何的参数 V1.0
===================================================================================*/
//对比字符串
u8 CHS_StrCmp(u8*str1,u8 *str2)
{
	while(1)
	{
		if(*str1!=*str2)return 0;//不相等
		if(*str1=='\0')break;//对比完成了.
		str1++;
		str2++;
	}
	return 1;//两个字符串相等
}
//把str1的内容copy到str2  
void CHS_StrCopy(u8*SercStr,u8 *DestStr)
{
	while(1)
	{										   
		*DestStr=*SercStr;	//拷贝
		if(*SercStr=='\0')break;//拷贝完成了.
		SercStr++;
		DestStr++;
	}
}

void CHS_MemCopy(u8*Serc,u8 *Dest,u16 Num)
{
	while(Num--)
	{										   
		*Dest=*Serc;	//拷贝
		Serc++;
		Dest++;
	}
}
//得到字符串的长度(字节) 
u8 CHS_StrLen(u8*str)
{
	u8 len=0;
	while(1)
	{							 
		if(*str=='\0')break;//拷贝完成了.
		len++;
		str++;
	}
	return len;
}
//返回值:m^n次方
u32 CHS_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}	   
//清空字符串
u8 CHS_ClearStr(u8* str,u8 Num)
{
	while(Num--)
	{
		*str=' ';
		str++;
	}
	return 0;
}
//把字符串转为数字
//支持16进制转换,但是16进制字母必须是大写的,且格式为以0X开头的.
//不支持负数 
//*str:数字字符串指针
//*res:转换完的结果存放地址.
//返回值:0，成功转换完成.其他,错误代码.
//1,数据格式错误.2,16进制位数为0.3,起始格式错误.4,十进制位数为0.
u8 CHS_Str2Num(u8*str,u32 *res)
{
	u32 t;
	u8 bnum=0;	 //数字的位数
	u8 *p;		  
	u8 hexdec=10;//默认为十进制数据
	p=str;
	*res=0;//清零.
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//参数合法
		{
			if(*p>='A')hexdec=16;	//字符串中存在字母,为16进制格式.
			bnum++;					//位数增加.
		}else if(*p=='\0')break;	//碰到结束符,退出.
		else return 1;				//不全是十进制或者16进制数据.
		p++; 
	} 
	p=str;			    //重新定位到字符串开始的地址.
	if(hexdec==16)		//16进制数据
	{
		if(bnum<3)return 2;			//位数小于3，直接退出.因为0X就占了2个,如果0X后面不跟数据,则该数据非法.
		if(*p=='0' && (*(p+1)=='X'))//必须以'0X'开头.
		{
			p+=2;	//偏移到数据起始地址.
			bnum-=2;//减去偏移量	 
		}else return 3;//起始头的格式不对
	}else if(bnum==0)return 4;//位数为0，直接退出.	  
	while(1)
	{
		if(bnum)bnum--;
		if(*p<='9'&&*p>='0')t=*p-'0';	//得到数字的值
		else t=*p-'A'+10;				//得到A~F对应的值	    
		*res+=t*CHS_Pow(hexdec,bnum);		   
		p++;
		if(*p=='\0')break;//数据都查完了.	
	}
	return 0;//成功转换
}

//查找函数的参数 传入函数的参数字符串
//返回 参数的个数
u8 CHS_GetParmNum(u8* str)
{
	u8 ParmNum=1;
	u8 IfStr=0;
	u8 ZeroParmTable1[]="()";
	u8 ZeroParmTable2[]="(void)";
	// 0参数判断
	if(CHS_StrCmp(str,ZeroParmTable1) || CHS_StrCmp(str,ZeroParmTable2)) return 0;
	while(1)
	{
		if((*str==')'||*str=='\0')) break;
		if(*str==',' && IfStr==0) ParmNum++;
		if(*str=='"' && IfStr==1)
		{
			IfStr=0;
		}
		else if(*str=='"') IfStr=1;
		str++;
	}
	return ParmNum;
}

//分析参数信息
//只能用于分析接受到函数参数信息 不能分析声明
//处理参数信息
u8 CHS_ParmProcess(u8* str,struct T_CHS_DEV* CHS_Dev)
{
	u8 i;
	u8 ParmPos[MAX_PARM];
	u8 Pos=0;
	u8 Temp[PARM_LEN];
	u32 NumParm;
	u8 Results;

	//第一步 给出各个参数在Buffer中的位置
	for(i=1;i<CHS_Dev->pnum+1;i++)
	{
		if(CHS_Dev->PInfo.Type[i]==2)
		{
			ParmPos[i]=Pos;
			Pos+=CHS_Dev->PInfo.Long[i]+2;//只存储字符串
		}
	}
  //存入数字
	for(i=1;i<CHS_Dev->pnum+1;i++)
	{
		if(CHS_Dev->PInfo.Type[i]==1) //数字
		{
			CHS_StrCopy(str+CHS_Dev->PInfo.OffSet[i],Temp);
			Temp[CHS_Dev->PInfo.Long[i]]='\0';
			Results=CHS_Str2Num(Temp,&NumParm);
			if(Results!=0) return 1;
			CHS_Dev->PInfo.ParmData[i]=NumParm;
		}
		else //字符串
		{
			CHS_StrCopy(str+CHS_Dev->PInfo.OffSet[i],Temp);
			Temp[CHS_Dev->PInfo.Long[i]]='\0';
			CHS_StrCopy(Temp,CHS_Dev->PInfo.ParmBuf+ParmPos[i]);
			CHS_Dev->PInfo.ParmData[i]=(u32)&(CHS_Dev->PInfo.ParmBuf)+ParmPos[i];
		}
	}
	return 0;
}
//获得函数参数在参数字符串中的偏移和长度
u8 CHS_GetParmInfo(u8* str,struct T_CHS_DEV* CHS_Dev)
{
	u8 i=0;
	u8 j=0;
	u8 ParmNum=1;
	u8 IfStr=0;
	u8 len;
	u8* OrgStrPtr=str;
	//获得参数个数
	CHS_Dev->PInfo.OffSet[1]=1;
	while(1)
	{
		if((*str==')'||*str=='\0')) break;
		if(*str==',' && IfStr==0)
		{
			ParmNum++;
			CHS_Dev->PInfo.OffSet[ParmNum]=i+1;
		}
		if((*str>='0' && *str<='9') && IfStr==0)
		{
			CHS_Dev->PInfo.Type[ParmNum]=1;
		}
		if(*str=='"' && IfStr==1)
		{
				IfStr=0;
		}
		else if(*str=='"')//碰到" 开始接收string
		{
			IfStr=1;
			CHS_Dev->PInfo.Type[ParmNum]=2; //确认是字符串参数
		}
		str++;
		i++;
		
	}
	//判断参数的开始位置
	for(i=1;i<ParmNum+1;i++)
	{
		if(CHS_Dev->PInfo.Type[i]==2)CHS_Dev->PInfo.OffSet[i]++;
	}
	//开始位置寻找结束
	i=0;
	str=OrgStrPtr;       //还原字符串
	len=CHS_StrLen(str); //获得字符串长度
	//获得每个参数的长度
	for(i=1;i<ParmNum+1;i++)
	{
		str=OrgStrPtr+CHS_Dev->PInfo.OffSet[i];
		for(j=CHS_Dev->PInfo.OffSet[i];j<len;j++)
		{
			if(CHS_Dev->PInfo.Type[i]==1)
			{
				if(*str==',' || *str=='"' || *str==')') //寻找结束标志
				{
					CHS_Dev->PInfo.Long[i]=j-CHS_Dev->PInfo.OffSet[i];
					break;	
				}
			}
			else
			{
				if(*str=='"')
				{
					CHS_Dev->PInfo.Long[i]=j-CHS_Dev->PInfo.OffSet[i]; //记录长度信息
					break;	
				}
			}
			str++;
		}
	}
	return 0;
	//将参数变形，存入Buffer并标记
}
//检查字符串是否合法 并且分离函数名字符串和参数字符串
//str     源字符串
//FunName 函数名称字符串
//FunParm 函数参数字符串
u8 CHS_StrCheck(u8* str,u8* FunName,u8* FunParm)
{
	u8 len;
	u8* StrStartPtr=str; //源字符串指针
	u8* p;
	u8 i=0;
	u8 FoverLeft=0;   //括号的个数
	u8 FoverRight=0;  //括号的个数
	u8 SpaceOffset=0; //第一个空格前的偏移
	//长度判断
	len=CHS_StrLen(str);
	if(len>(PARM_LEN+MAX_FNAME_LEN)) return 1;
	//括号判断
	for(i=0;i<len;i++)
	{
		if(*str=='(')  FoverLeft++;
		if(*str==')')  FoverRight++;
		str++;
	}
	if(FoverLeft==0 || FoverRight==0 || FoverLeft!=FoverRight)  return 4;
	str=StrStartPtr; //还原字符串指针
	
	for(i=0;i<len;i++)
	{
		if(*str=='(') break;
		str++;
	}
	FoverLeft=i;
	//跳过第一个空格，检测第一个字母是否为合法字母
	str=StrStartPtr;
	for(i=0;i<len;i++)
	{
		if(*str==' ' && (i<FoverLeft)) SpaceOffset=i+1;
		str++;
	}
	str=StrStartPtr;
	str=(str+SpaceOffset);
	p=str;
	if(!((*str>='a' && *str<='z')||(*str>='A' && *str<='Z')|| (*str=='_'))) return 5;
	//寻找第一个(
	str=StrStartPtr;
	for(i=0;i<len;i++)
	{
		if(*str=='(') break;
		str++;
	}
	str=p;
	CHS_MemCopy(str,FunName,i); //拷贝FunName字符串
	CHS_StrCopy(str+i-SpaceOffset,FunParm);
	*(FunName+i-SpaceOffset)='\0';
	if(CHS_StrLen(FunName)>MAX_FNAME_LEN) return 6; //检测长度是否超出最大
	if(CHS_StrLen(FunParm)>PARM_LEN)      return 6;
	return 0;
}
//命令表
u8 *CHS_CMD_TABLE[]=
{
	"?",
	"help",
	"list",
};	
//主执行函数
u8  CHS_Scan()
{
	u8 i=0;
	u8 Result=0;
	u8 len=0;
	u8 LocalFunName[MAX_FNAME_LEN];   //本地函数名称字符串
	u8 LocalFunParm[PARM_LEN];        //本地参数名称字符串
	u8 RecFunName[MAX_FNAME_LEN];     //接收到的函数名称字符串
	u8 RecFunParm[PARM_LEN];          //接收到的函数参数字符串
	if((CHS_Dev.USART_STAT&0x8000)==0) return 1;
	len=CHS_Dev.USART_STAT&0x3fff;	 //得到此次接收到的数据长度
	CHS_Dev.USART_BUF[len]='\0';	   //在末尾加入结束符. 
	Result=CHS_StrCheck(CHS_Dev.USART_BUF,RecFunName,RecFunParm); //检测输入字符串是否为函数格式，如果是，分离名称和参数字符串
	if(Result!=0) //不是C语言函数格式的输入
	{
		for(i=0;i<3;i++)//支持3个系统指令
		{
			if(CHS_StrCmp(CHS_Dev.USART_BUF,CHS_CMD_TABLE[i]))break;
		}
		switch(i)
		{					   
			case 0:
			case 1://帮助指令
				printf("--------------------------CH SHELL V1.0------------------------ \r\n");
				printf("    CHSHELL是由YANDLD开发的一个灵巧的串口调试互交组件,通过它,你 \r\n");
				printf("可以通过串口助手调用程序里面的任何函数,并执行.因此,你可以随意更改\r\n");
				printf("函数的输入参数.单个函数最多支持6个输入参数.可以是数字或者字符串  \r\n");	  
				printf("技术支持:www.tinychip.net\r\n");
				printf("CH SHELL有3个系统命令:\r\n"); 
				printf("?:    获取帮助信息\r\n");
				printf("help: 获取帮助信息\r\n");
				printf("list: 可用的函数列表\r\n\n");
				printf("请按照程序编写格式输入函数名及参数并以回车键结束.\r\n");    
				printf("--------------------------YANDLD ------------------------- \r\n");
			case 2:
				printf("\r\n");
				printf("-------------------------函数清单--------------------------- \r\n");
				for(i=0;i<CHS_Dev.fnum;i++)printf("%s\r\n",CHS_Dev.funs[i].name);
				printf("\r\n");
				return 0;
			default: 
				printf("无法识别的指令\r\n");
				return 1;
		}
	}
	  //检查本地是否有此函数
		for(i=0;i<CHS_Dev.fnum;i++) 
		{
			Result=CHS_StrCheck((u8*)CHS_Dev.funs[i].name,LocalFunName,LocalFunParm);//得到本地函数名及参数个数
			if(Result!=0)
			{
				printf("本地函数解析错误 %d\r\n",Result);
				return 2;//本地解析有误
			}
			if(CHS_StrCmp(LocalFunName,RecFunName))//输入函数名和本地函数名匹配
			{
				CHS_Dev.id=i;//记录函数ID.
				break;//跳出.
			}	
		}
		if(i==CHS_Dev.fnum) //输入函数名和本地函数名无法匹配
		{
			printf("无法匹配函数\r\n");
			return 5;
		}
		//检测参数输入是否正确
		if(CHS_GetParmNum(RecFunParm)!=CHS_GetParmNum(LocalFunParm))
		{
			printf("参数输入不正确 本地%d 输入%d\r\n",CHS_GetParmNum(LocalFunParm),CHS_GetParmNum(RecFunParm));
			return 6;
		}
		
		CHS_Dev.pnum=CHS_GetParmNum(RecFunParm); //获取此次要执行函数的参数个数
		CHS_GetParmInfo(RecFunParm,&CHS_Dev);    //获得参数信息偏移和大小
		CHS_ParmProcess(RecFunParm,&CHS_Dev);   //处理函数信息
		//检查正确 开始分析参数
		printf("\r\n%s(",RecFunName);//输出正要执行的函数名
		for(i=1;i<CHS_Dev.pnum+1;i++) //输入要执行的参数
		{
			if(CHS_Dev.PInfo.Type[i]==1) //数字
			{
				printf("%d",CHS_Dev.PInfo.ParmData[i]);
			}
			else  //字符串
			{
				printf("%c",'"');			 
				printf("%s",(u8*)CHS_Dev.PInfo.ParmData[i]);
				printf("%c",'"');
			}
			if(i!=CHS_Dev.pnum)printf(",");
		}
		printf(")\r\n");	
		//开始执行
		switch(CHS_Dev.pnum)
		{
			case 0://无参数(void类型)											  
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)();
				printf("FunRet: %d",Result);
				break;
	    case 1://有1个参数
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)(CHS_Dev.PInfo.ParmData[1]);
				printf("FunRet: %d",Result);
				break;
			case 2: //有2个参数
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)(CHS_Dev.PInfo.ParmData[1],CHS_Dev.PInfo.ParmData[2]);
				printf("FunRet: %d",Result);
				break;
			case 3: //有3个参数
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)(CHS_Dev.PInfo.ParmData[1],CHS_Dev.PInfo.ParmData[2],CHS_Dev.PInfo.ParmData[3]);
				printf("FunRet: %d",Result);
				break;
			case 4: //有4个参数
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)(CHS_Dev.PInfo.ParmData[1],CHS_Dev.PInfo.ParmData[2],CHS_Dev.PInfo.ParmData[3],CHS_Dev.PInfo.ParmData[4]);
				printf("FunRet: %d",Result);
				break;		
			case 5: //有5个参数
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)(CHS_Dev.PInfo.ParmData[1],CHS_Dev.PInfo.ParmData[2],CHS_Dev.PInfo.ParmData[3],CHS_Dev.PInfo.ParmData[4],\
				CHS_Dev.PInfo.ParmData[5]);
				printf("FunRet: %d",Result);
				break;		
			case 6: //有6个参数
				Result=(*(u32(*)())CHS_Dev.funs[CHS_Dev.id].func)(CHS_Dev.PInfo.ParmData[1],CHS_Dev.PInfo.ParmData[2],CHS_Dev.PInfo.ParmData[3],CHS_Dev.PInfo.ParmData[4],\
				CHS_Dev.PInfo.ParmData[5],CHS_Dev.PInfo.ParmData[6]);
				printf("FunRet: %d",Result);
				break;	
			default :
				printf("输入错误\r\n");
				break;
		}
		return 0;
}

//接收1个字符串 如果接收成功，调用scan() 函数一次
void CHS_Rev1(u8 ch)
{
	if((CHS_Dev.USART_STAT&0x8000)==0)
	{
		if(CHS_Dev.USART_STAT&0x4000) //接收到了0x0D
		{
			if(ch!=0x0A)  CHS_Dev.USART_STAT=0; //接收错误
			else  
			{
				CHS_Dev.USART_STAT|=0x8000;    //接收完成
				CHS_Scan();                    //执行主程序
				CHS_ClearStr(CHS_Dev.USART_BUF,sizeof(CHS_Dev.USART_BUF));
				CHS_Dev.USART_STAT=0;
			}
		}
		else 
		{
			if(ch==0x0d) CHS_Dev.USART_STAT|=0x4000;//接收到了回车键
			else
			{
				CHS_Dev.USART_BUF[CHS_Dev.USART_STAT&0x3FFF]=ch;
				CHS_Dev.USART_STAT++;
				if((CHS_Dev.USART_STAT&0x3FFF)>(MAX_FNAME_LEN+PARM_LEN-1)) CHS_Dev.USART_STAT=0; //接收数据错误,重新开始接收	  
			}		
		}
	}
}

