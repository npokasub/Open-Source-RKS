#include <printf.h>

#include "TimeLib.h"


void Hdot();
void hc595senddata(byte data,byte data2,byte data3,byte data4);//发送上下半屏，各一行数据。
void hc138sacn(byte r);   //输出行线状态ABCD （A低,D高)
void DataUpdate();
bool GpsUpdate(unsigned char k);
void PmUpdate(unsigned char   k  );
void PmLog();//every 5 mins
void PmUpdateChart();

bool _esp8266_waitFor(const char *string);
bool _esp8266_getch(char * RetData);
bool _esp8266_getValue(const char *string,unsigned char  * Value,unsigned char * Len,unsigned char Start,unsigned char Max) ;
bool TimeRequest(unsigned char k);
void HourlyUpdate();
void OnSeconds();
void NonStopTask();

void StartNTP();
void CheckNTP();
bool bNTPRunning = false;


bool GetNow();
bool GetAir();
bool GetForcast(unsigned char Day,unsigned char Postion);

void ShowChart(unsigned char ChartIndex,unsigned char PostionX,unsigned char PostionY);

void WeatherCode2ChartIndex(unsigned char Code,unsigned char * ChartIndex);


#define TIME_OUT 30


#define WIFI_SERIAL Serial3



#define RowA 45    		//行信号,驱动138  
#define RowB 46
#define RowC 47
#define RowD 48
#define STB 49         //595 刷新显示  SS  
#define CLK 50         //时钟    SCK  
#define OE 51 			//  使能  
#define R1 52          //上半屏列信号输出    
#define R2 53          //下半屏列信号输出     

#define R3 44          //上半屏列信号输出    
#define R4 43          //下半屏列信号输出    

//Display

unsigned char ASCII816Buf[16] = {0};
unsigned char DispBuf[16][32] = {0};
unsigned char TimeLine[8]= {32,32,32,32,32,32,32,32};
//unsigned char SecondLine[8]= {32,32,32,32,32,32,32,32};//{'A','B','C','D','H','I','O','P'};
//unsigned char FirstLine[16]= {'A','B','C','D','H','I','O','P','P','P','P','P','P','P','P','P'};
unsigned char DateLine[10]= {32,32,32,32,32,32,32,32,32,32};//{'1','2','3','4','5','6','7','8','8',' '};//{'A','B','C','D','H','I','O','P'};
unsigned char DateLine2[10] = {32,32,32,32,32,32,32,32,32,32};//{'w','e','a','t','h','e','r','n','o','w'};

unsigned char WeatherDay[2][10] = {{32,32,32,32,32,32,32,32,32,32},{32,32,32,32,32,32,32,32,32,32}};//{'d','a','y','1',' ',' ','T','e','m','p'};
unsigned char Weather[2][10] = {{32,32,32,32,32,32,32,32,32,32},{32,32,32,32,32,32,32,32,32,32}};//{'w','e','a','t','h','e','r','n','T','p'};

unsigned char WeatherCode[2][3];

//Serial data
int comdata ;

//GPS


unsigned char GPRMC_command = 0;
unsigned char GPRMC_command_array[6] =  {'$','G','P','R','M','C'};
unsigned char GPRMC_Index = 0;
unsigned char GPS_Time[6];
unsigned char GPS_Date[6];
unsigned char GPS_Valid[1];
unsigned char ElementIndex = 0;

unsigned char Time_request_array[10] =  {'+','I','P','D',',','1',',','4','8',':'};
tmElements_t tm;
time_t t;
bool bGPS_Valid;

//const char Http[]   ="GET /Aqi/LookUp?key=7e24c5e18e054287907124acf2b1aa9f&city=%E5%8D%97%E4%BA%AC HTTP/1.1\r\nHost: api.avatardata.cn\r\nUser-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.7.6)\r\nConnection: Keep-Alive\r\n\r\n";

//const char Http[]   ="GET /Aqi/LookUp?key=7e24c5e18e054287907124acf2b1aa9f&city=%E5%8D%97%E4%BA%AC HTTP/1.1\r\nHost: api.avatardata.cn\r\n\r\n";

//const char Http[]   ="GET /Weather/Query?key=ed1055e2f14c4ba08a11dc4e941f7180&cityname=%E5%8D%97%E4%BA%AC&format=true HTTP/1.1\r\nHost: api.avatardata.cn\r\n\r\n";


const char HttpNow[]   ="GET /v3/weather/now.json?key=sxegxyiedpdmhqhl&location=nanjing&language=en&unit=c HTTP/1.1\r\nHost: api.seniverse.com\r\n\r\n";
char HttpForcast[]   ="GET /v3/weather/daily.json?key=sxegxyiedpdmhqhl&location=nanjing&language=en&unit=c&start=0&days=1 HTTP/1.1\r\nHost: api.seniverse.com\r\n\r\n";
char HttpAir[]   ="GET /s6/air/now?location=nanjing&key=71dad6c640ee40199c45d9cbe997921e&lang=en HTTP/1.1\r\nHost: free-api.heweather.com\r\n\r\n";

unsigned char WeekStr[] = "SunMonTueWedThuFriSat";

unsigned char InfoLen;


unsigned long SecondsSinceStart = 0;


//晴多云阴阵雨雷小中大暴雪冻浮尘扬沙雾霾风飓龙卷冷热未知

#define	KONG		0
#define	QING	1
#define	DUO	2
#define	YUN	3
#define	YING	4
#define	ZHEN	5
#define	YU	6
#define	LEI	7
#define	XIAO	8
#define	ZHONG	9
#define	DA	10
#define	BAO	11
#define	XUE	12
#define	DONG	13
#define	FU	14
#define	CHENG	15
#define	YANG	16
#define	SHA	17
#define	WU	18
#define	MAI	19
#define	FENG	20
#define	JU	21
#define	LONG	22
#define	JUAN	23
#define	LENG	24
#define	RE	25
#define	WEI	26
#define	ZHI	27



unsigned char Chart[] = {

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x40,0x08,0x48,0x7F,0xFC,0x48,0x40,0x4B,0xF8,0x48,0x40,0x4F,0xFE,0x78,0x08,0x4B,0xFC,0x4A,0x08,0x4B,0xF8,0x4A,0x08,0x7B,0xF8,0x4A,0x08,0x02,0x28,0x02,0x10,

	0x02,0x00,0x02,0x00,0x07,0xF0,0x08,0x20,0x18,0x40,0x25,0x80,0x02,0x80,0x0C,0x80,0x71,0xFC,0x02,0x08,0x0C,0x10,0x12,0x20,0x21,0xC0,0x01,0x00,0x0E,0x00,0x70,0x00,

	0x00,0x00,0x00,0x20,0x1F,0xF0,0x00,0x00,0x00,0x00,0x00,0x04,0xFF,0xFE,0x02,0x00,0x02,0x00,0x04,0x00,0x04,0x00,0x08,0x00,0x08,0x20,0x10,0x10,0x3F,0xF8,0x00,0x08,

	0x00,0x04,0x7C,0xFE,0x44,0x84,0x48,0x84,0x48,0x84,0x50,0xFC,0x48,0x84,0x48,0x84,0x44,0x84,0x44,0xFC,0x44,0x84,0x68,0x84,0x51,0x04,0x41,0x04,0x42,0x14,0x44,0x08,

	0x00,0x80,0x78,0x84,0x4F,0xFE,0x50,0x80,0x50,0xA0,0x61,0x20,0x51,0x28,0x4B,0xFC,0x48,0x20,0x48,0x20,0x68,0x24,0x57,0xFE,0x40,0x20,0x40,0x20,0x40,0x20,0x40,0x20,

	0x00,0x04,0xFF,0xFE,0x01,0x00,0x01,0x08,0x3F,0xFC,0x21,0x08,0x21,0x08,0x29,0x48,0x25,0x28,0x21,0x08,0x21,0x08,0x29,0x48,0x25,0x28,0x21,0x08,0x21,0x28,0x20,0x10,

	0x00,0x10,0x3F,0xF8,0x01,0x00,0x7F,0xFE,0x41,0x02,0x9D,0x74,0x01,0x00,0x1D,0x70,0x01,0x00,0x3F,0xF8,0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08,0x21,0x08,0x3F,0xF8,

	0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x05,0x40,0x05,0x20,0x09,0x10,0x09,0x08,0x11,0x04,0x21,0x04,0x41,0x00,0x01,0x00,0x01,0x00,0x05,0x00,0x02,0x00,

	0x01,0x00,0x01,0x00,0x01,0x04,0x7F,0xFE,0x41,0x04,0x41,0x04,0x41,0x04,0x41,0x04,0x7F,0xFC,0x41,0x04,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,

	0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x04,0xFF,0xFE,0x01,0x00,0x02,0x80,0x02,0x80,0x02,0x40,0x04,0x40,0x04,0x20,0x08,0x10,0x10,0x0E,0x60,0x04,0x00,0x00,

	0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x04,0x40,0x7F,0xFC,0x04,0x40,0xFF,0xFE,0x04,0x40,0x09,0x20,0x35,0x5E,0xC3,0x84,0x09,0x20,0x11,0x10,0x03,0x00,

	0x1F,0xF0,0x01,0x00,0x7F,0xFE,0x41,0x02,0x9D,0x74,0x01,0x00,0x1D,0x70,0x01,0x08,0x3F,0xFC,0x00,0x08,0x00,0x08,0x1F,0xF8,0x00,0x08,0x00,0x08,0x3F,0xF8,0x00,0x08,

	0x00,0x40,0x40,0x48,0x23,0xFC,0x10,0x80,0x10,0x80,0x01,0x20,0x01,0x24,0x0B,0xFE,0x10,0x20,0x60,0xA0,0x20,0xA8,0x21,0x24,0x22,0x22,0x24,0x22,0x20,0xA0,0x00,0x40,

	0x40,0x08,0x30,0x3C,0x17,0xC0,0x00,0x08,0x82,0x48,0x62,0x50,0x20,0x00,0x0B,0xF8,0x10,0x20,0x20,0x44,0xEF,0xFE,0x20,0x40,0x20,0x40,0x20,0x40,0x21,0x40,0x20,0x80,

	0x01,0x00,0x01,0x00,0x05,0x40,0x0D,0x20,0x11,0x18,0x21,0x0C,0x41,0x04,0x00,0x00,0x01,0x00,0x01,0x08,0x7F,0xFC,0x01,0x00,0x01,0x00,0x01,0x04,0xFF,0xFE,0x00,0x00,

	0x10,0x00,0x13,0xF8,0x10,0x10,0x10,0x20,0xFC,0x40,0x10,0x84,0x17,0xFE,0x18,0xA4,0x30,0xA4,0xD0,0xA4,0x11,0x24,0x11,0x44,0x12,0x44,0x14,0x84,0x51,0x28,0x20,0x10,

	0x00,0x40,0x20,0x40,0x18,0x40,0x08,0x40,0x81,0x50,0x61,0x48,0x22,0x46,0x0A,0x42,0x14,0x48,0x20,0x48,0xE0,0x50,0x20,0x20,0x20,0x40,0x20,0x80,0x23,0x00,0x2C,0x00,

	0x3F,0xF8,0x01,0x00,0x7F,0xFE,0x51,0x12,0x89,0x24,0x14,0x10,0x07,0xE0,0x0C,0x40,0x13,0x80,0x0C,0x60,0xF2,0x1E,0x0F,0xE4,0x02,0x20,0x04,0x20,0x08,0xA0,0x30,0x40,

	0x1F,0xF0,0x01,0x00,0x7F,0xFE,0x51,0x12,0x8D,0x64,0x10,0x10,0x2B,0xFC,0xE5,0x24,0x19,0xFC,0x69,0x24,0x95,0xFC,0x64,0x20,0x05,0xFC,0x64,0x20,0x17,0xFE,0x08,0x00,

	0x00,0x10,0x3F,0xF8,0x20,0x10,0x28,0x50,0x24,0x50,0x22,0x90,0x22,0x90,0x21,0x10,0x21,0x10,0x22,0x90,0x22,0x90,0x24,0x50,0x28,0x50,0x30,0x12,0x40,0x0A,0x80,0x06,

	0x04,0x08,0x7E,0xFC,0x44,0x88,0x44,0xF8,0x44,0x88,0x6C,0xF8,0x6C,0x88,0x54,0xF8,0x54,0x88,0x54,0x88,0x6F,0xFE,0x6C,0x50,0x44,0x8A,0x44,0x02,0x43,0xFE,0x80,0x00,

	0x02,0x00,0x02,0x40,0x02,0x20,0x02,0x04,0xFF,0xFE,0x02,0x80,0x02,0x88,0x04,0x88,0x04,0x90,0x04,0xA0,0x08,0xC0,0x08,0x82,0x11,0x82,0x16,0x82,0x20,0x7E,0x40,0x00,

	0x01,0x00,0x11,0x10,0x09,0x20,0x01,0x08,0x7F,0xFC,0x02,0x80,0xFF,0xFE,0x04,0x40,0x08,0x20,0x1F,0xF0,0x28,0x2E,0xC8,0x24,0x09,0x20,0x08,0xC8,0x08,0x08,0x07,0xF8,

	0x00,0x80,0x40,0x80,0x31,0x40,0x12,0x20,0x04,0x10,0x09,0x08,0x10,0x8E,0x10,0x84,0x27,0xF0,0xE0,0x10,0x20,0x20,0x20,0x40,0x22,0x80,0x21,0x00,0x20,0xC0,0x00,0x40,

	0x08,0x40,0x08,0x40,0x08,0x48,0x7F,0xFC,0x08,0x48,0x0A,0x48,0x1C,0xC8,0x68,0x48,0x08,0xA8,0x08,0x8A,0x29,0x0A,0x12,0x04,0x00,0x00,0x48,0x88,0x44,0x46,0x84,0x42,

	0x01,0x00,0x01,0x00,0x01,0x08,0x7F,0xFC,0x01,0x00,0x01,0x00,0x01,0x04,0xFF,0xFE,0x03,0x80,0x05,0x40,0x09,0x20,0x11,0x10,0x21,0x0E,0x41,0x04,0x01,0x00,0x01,0x00,

	0x20,0x00,0x20,0x00,0x22,0x04,0x3F,0x7E,0x28,0x44,0x48,0x44,0x88,0x44,0x09,0x44,0xFF,0xC4,0x08,0x44,0x08,0x44,0x14,0x44,0x12,0x44,0x22,0x7C,0x40,0x44,0x80,0x00,

};

//http://blog.csdn.net/zhouyanldh/article/details/8558961
/*
显示流程:
1:传送595数据
2:关闭EN 1，关闭显示
3:切换行信息
4:刷新595，STB
5：开EN 0
6:延时 (刷新率调整)
*/

//8*16H

//http://wenku.baidu.com/link?url=QXqXkJc10ap3VpSeMUoc5c5Z9dMRwLWYMT_zRjMt70xYbIwV-IRLw4J59QrHo7bLuBy5PSHgbAI3n-RkPvE0cEqrEl7FY-POdne7dChx0Vy


/***********************************************xyz
ASCII纵向取模8*16字库。为1亮。
纵向取模（8位）(取完一横后再取下一横)-LCD12864SPI
每个字节代表一列8个像素值，每个ASCII由8*16/8=16个字节存储，前8个字节为上半部分值，后8个字节为下半部分值。
共95个，从32(空格)到126。ASCII816[0]为空格。
ASCII值减32即为数组二维下标。
**************************************************/
unsigned char ASCII816[96][16]=
{
	// !
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00,
	//"#
	0x00,0x10,0x0C,0x06,0x10,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x40,0xC0,0x78,0x40,0xC0,0x78,0x40,0x00,0x04,0x3F,0x04,0x04,0x3F,0x04,0x04,0x00,
	//$% 

	0x00,0x70,0x88,0xFC,0x08,0x30,0x00,0x00,0x00,0x18,0x20,0xFF,0x21,0x1E,0x00,0x00,
	0xF0,0x08,0xF0,0x00,0xE0,0x18,0x00,0x00,0x00,0x21,0x1C,0x03,0x1E,0x21,0x1E,0x00,
	//&'

	0x00,0xF0,0x08,0x88,0x70,0x00,0x00,0x00,0x1E,0x21,0x23,0x24,0x19,0x27,0x21,0x10,
	0x10,0x16,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//() 
	0x00,0x00,0x00,0xE0,0x18,0x04,0x02,0x00,0x00,0x00,0x00,0x07,0x18,0x20,0x40,0x00,
	0x00,0x02,0x04,0x18,0xE0,0x00,0x00,0x00,0x00,0x40,0x20,0x18,0x07,0x00,0x00,0x00,
	//*+

	0x40,0x40,0x80,0xF0,0x80,0x40,0x40,0x00,0x02,0x02,0x01,0x0F,0x01,0x02,0x02,0x00,
	0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x1F,0x01,0x01,0x01,0x00,
	//,-

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xB0,0x70,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	//./ 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x04,0x00,0x60,0x18,0x06,0x01,0x00,0x00,0x00,
	//01


	0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00,
	0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,
	//23

	0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00,
	0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00,
	//45
	0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00,
	0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00,
	//67
	0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00,
	0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00,
	//89
	0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00,
	0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00,
	//:;

	0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,
	0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x00,
	//<=
	0x00,0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x00,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,
	//>?
	0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,0x01,0x00,
	0x00,0x70,0x48,0x08,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x30,0x36,0x01,0x00,0x00,
	//@A
	0xC0,0x30,0xC8,0x28,0xE8,0x10,0xE0,0x00,0x07,0x18,0x27,0x24,0x23,0x14,0x0B,0x00,
	0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20,
	//BC
	0x08,0xF8,0x88,0x88,0x88,0x70,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00,
	0xC0,0x30,0x08,0x08,0x08,0x08,0x38,0x00,0x07,0x18,0x20,0x20,0x20,0x10,0x08,0x00,
	//DE
	0x08,0xF8,0x08,0x08,0x08,0x10,0xE0,0x00,0x20,0x3F,0x20,0x20,0x20,0x10,0x0F,0x00,
	0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x20,0x23,0x20,0x18,0x00,
	//FG?

	0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x00,0x03,0x00,0x00,0x00,
	0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00,0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00,
	//HI
	0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x20,0x3F,0x21,0x01,0x01,0x21,0x3F,0x20,
	0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,
	//JK
	0x00,0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,0x00,
	0x08,0xF8,0x88,0xC0,0x28,0x18,0x08,0x00,0x20,0x3F,0x20,0x01,0x26,0x38,0x20,0x00,
	//LM
	0x08,0xF8,0x08,0x00,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x20,0x30,0x00,
	0x08,0xF8,0xF8,0x00,0xF8,0xF8,0x08,0x00,0x20,0x3F,0x00,0x3F,0x00,0x3F,0x20,0x00,
	//NO
	0x08,0xF8,0x30,0xC0,0x00,0x08,0xF8,0x08,0x20,0x3F,0x20,0x00,0x07,0x18,0x3F,0x00,
	0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x10,0x20,0x20,0x20,0x10,0x0F,0x00,
	//PQ
	0x08,0xF8,0x08,0x08,0x08,0x08,0xF0,0x00,0x20,0x3F,0x21,0x01,0x01,0x01,0x00,0x00,
	0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x18,0x24,0x24,0x38,0x50,0x4F,0x00,
	//RS
	0x08,0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x00,0x03,0x0C,0x30,0x20,
	0x00,0x70,0x88,0x08,0x08,0x08,0x38,0x00,0x00,0x38,0x20,0x21,0x21,0x22,0x1C,0x00,
	//TU
	0x18,0x08,0x08,0xF8,0x08,0x08,0x18,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00,
	0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00,
	//VW
	0x08,0x78,0x88,0x00,0x00,0xC8,0x38,0x08,0x00,0x00,0x07,0x38,0x0E,0x01,0x00,0x00,
	0xF8,0x08,0x00,0xF8,0x00,0x08,0xF8,0x00,0x03,0x3C,0x07,0x00,0x07,0x3C,0x03,0x00,
	//XY
	0x08,0x18,0x68,0x80,0x80,0x68,0x18,0x08,0x20,0x30,0x2C,0x03,0x03,0x2C,0x30,0x20,
	0x08,0x38,0xC8,0x00,0xC8,0x38,0x08,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00,
	//Z[
	0x10,0x08,0x08,0x08,0xC8,0x38,0x08,0x00,0x20,0x38,0x26,0x21,0x20,0x20,0x18,0x00,
	0x00,0x00,0x00,0xFE,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x00,
	//\]
	0x00,0x0C,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x38,0xC0,0x00,
	0x00,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x7F,0x00,0x00,0x00,
	//^_

	0x00,0x00,0x04,0x02,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	//`a

	0x00,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x19,0x24,0x22,0x22,0x22,0x3F,0x20,
	//bc
	0x08,0xF8,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x3F,0x11,0x20,0x20,0x11,0x0E,0x00,
	0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x00,0x0E,0x11,0x20,0x20,0x20,0x11,0x00,
	//de
	0x00,0x00,0x00,0x80,0x80,0x88,0xF8,0x00,0x00,0x0E,0x11,0x20,0x20,0x10,0x3F,0x20,
	0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x22,0x22,0x22,0x22,0x13,0x00,
	//fg
	0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x18,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,
	0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x6B,0x94,0x94,0x94,0x93,0x60,0x00,
	//hi
	0x08,0xF8,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20,
	0x00,0x80,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,
	//jk

	0x00,0x00,0x00,0x80,0x98,0x98,0x00,0x00,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,
	0x08,0xF8,0x00,0x00,0x80,0x80,0x80,0x00,0x20,0x3F,0x24,0x02,0x2D,0x30,0x20,0x00,
	//lm
	0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x3F,0x20,0x00,0x3F,0x20,0x00,0x3F,
	//no
	0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20,
	0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00,
	//pq
	0x80,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0xFF,0xA1,0x20,0x20,0x11,0x0E,0x00,
	0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x0E,0x11,0x20,0x20,0xA0,0xFF,0x80,
	//rs
	0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x20,0x20,0x3F,0x21,0x20,0x00,0x01,0x00,
	0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x33,0x24,0x24,0x24,0x24,0x19,0x00,
	//tu?
	0x00,0x80,0x80,0xE0,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00,0x00,
	0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x1F,0x20,0x20,0x20,0x10,0x3F,0x20,
	//vw
	0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x00,0x01,0x0E,0x30,0x08,0x06,0x01,0x00,
	0x80,0x80,0x00,0x80,0x00,0x80,0x80,0x80,0x0F,0x30,0x0C,0x03,0x0C,0x30,0x0F,0x00,
	//xy
	0x00,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x31,0x2E,0x0E,0x31,0x20,0x00,
	0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,0x81,0x8E,0x70,0x18,0x06,0x01,0x00,
	//z{
	0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x21,0x30,0x2C,0x22,0x21,0x30,0x00,
	0x00,0x00,0x00,0x00,0x80,0x7C,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x3F,0x40,0x40,
	//|}
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0x00,0x02,0x02,0x7C,0x80,0x00,0x00,0x00,0x00,0x40,0x40,0x3F,0x00,0x00,0x00,0x00,
	//~
	0x00,0x06,0x01,0x01,0x02,0x02,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
//5*7点阵,纵向取模(高位在下方),字节正序(自左到右排列)


unsigned char ACSII57[95][5]=
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00 },  // sp
	{ 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
	{ 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
	{ 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
	{ 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
	{ 0xc4, 0xc8, 0x10, 0x26, 0x46 },   // %
	{ 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
	{ 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
	{ 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
	{ 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
	{ 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
	{ 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
	{ 0x00, 0x00, 0x50, 0x30, 0x00 },   // ,
	{ 0x10, 0x10, 0x10, 0x10, 0x10 },   // -
	{ 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
	{ 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
	{ 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
	{ 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
	{ 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
	{ 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
	{ 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
	{ 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
	{ 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
	{ 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
	{ 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
	{ 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
	{ 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
	{ 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
	{ 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
	{ 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
	{ 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
	{ 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
	{ 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
	{ 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A
	{ 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
	{ 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
	{ 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
	{ 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
	{ 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
	{ 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
	{ 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
	{ 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
	{ 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
	{ 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
	{ 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
	{ 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
	{ 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
	{ 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
	{ 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
	{ 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
	{ 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
	{ 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
	{ 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
	{ 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
	{ 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
	{ 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
	{ 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
	{ 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
	{ 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
	{ 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
	{ 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 55
	{ 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
	{ 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
	{ 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
	{ 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
	{ 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
	{ 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
	{ 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
	{ 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
	{ 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
	{ 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
	{ 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g
	{ 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
	{ 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
	{ 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j
	{ 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
	{ 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
	{ 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
	{ 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
	{ 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
	{ 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p
	{ 0x08, 0x14, 0x14, 0x18, 0x7C },   // q
	{ 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
	{ 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
	{ 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
	{ 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
	{ 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
	{ 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
	{ 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
	{ 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y
	{ 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
	{ 0x00, 0x08, 0x36, 0x41, 0x00 },   // {
	{ 0x00, 0x00, 0x77, 0x00, 0x00 },   // |
	{ 0x00, 0x41, 0x36, 0x08, 0x00 },   // }
	{ 0x04, 0x02, 0x04, 0x08, 0x04 }   // ~
};









void setup()
{


	Serial.begin(115200);
	Serial.println(F("GPS_Time"));
	printf_begin();


	//Serial.begin(115200);
	Serial1.begin(9600);
	Serial2.begin(38400);
	Serial3.begin(115200);



	pinMode(RowA, OUTPUT);
	pinMode(RowB, OUTPUT);
	pinMode(RowC, OUTPUT);
	pinMode(RowD, OUTPUT); //138片选
	pinMode(OE, OUTPUT); //138 使能
	pinMode(R1, OUTPUT);//595 数据
	pinMode(R2, OUTPUT);//595 数据
	pinMode(R3, OUTPUT);//595 数据
	pinMode(R4, OUTPUT);//595 数据
	pinMode(CLK, OUTPUT); //595 时钟
	pinMode(STB, OUTPUT); //595 使能


	DDRA=B00001111;



	Hdot();//字库取模改成横向

	WIFI_SERIAL.print(F("AT+RST\r\n"));
	_esp8266_waitFor("GOT IP\r\n");



	WIFI_SERIAL.print(F("AT+CIPSSLSIZE=4096\r\n"));
	_esp8266_waitFor("OK\r\n");


	StartNTP();

	HourlyUpdate();

}

void loop()
{
	//while (1)
	//{
	//	if (Serial3.available() > 0)
	//	{
	//		comdata = Serial3.read();
	//		Serial.write(comdata);
	//	}

	//	if (Serial.available() > 0)
	//	{
	//		comdata = Serial.read();
	//		Serial3.write(comdata);
	//	}
	//}


	//while (1)
	//{
	//	if (Serial2.available() > 0)
	//	{
	//		comdata = Serial2.read();
	//		Serial.write(comdata);
	//	}

	//	if (Serial.available() > 0)
	//	{
	//		comdata = Serial.read();
	//		Serial2.write(comdata);
	//	}
	//}





	NonStopTask();







}


void NonStopTask()
{

	//static unsigned char row=0;//16
	//static unsigned char col=0;//8
	////显示缓存扫描
	//for(row=0; row<16; row++)
	//{

	//	for (col=0; col<8; col++)
	//	{	
	//		hc595senddata(DispBuf[col][row],DispBuf[col][row+16],DispBuf[col+8][row],DispBuf[col+8][row+16]);//发送列数据，上16行与下16行同时发送。
	//	}
	//	digitalWrite(OE, 1);  //关闭显示
	//	hc138sacn(row);            //选行
	//	digitalWrite(STB, 1);      //数据确认
	//	digitalWrite(STB, 0);
	//	DataUpdate();
	//	CheckNTP();
	//	digitalWrite(OE, 0);  //开启显示

	//}


	static unsigned char row=0;//16
	static unsigned char col=0;//8
	//显示缓存扫描
	hc595senddata(DispBuf[col][row],DispBuf[col][row+16],DispBuf[col+8][row],DispBuf[col+8][row+16]);//发送列数据，上16行与下16行同时发送。
	col++;
	if (col>=8)
	{
		digitalWrite(OE, 1);  //关闭显示
		hc138sacn(row);            //选行
		digitalWrite(STB, 1);      //数据确认
		digitalWrite(STB, 0);
		DataUpdate();
		CheckNTP();
		digitalWrite(OE, 0);  //开启显示

		col=0;
		row++;
		if (row>=16)
		{
			row = 0;
		}
	}

}



void StartNTP()
{

	//WIFI_SERIAL.print(F("AT+CIPSTART=1,\"UDP\",\"192.168.0.12\",123,123,2\r\n"));
	WIFI_SERIAL.print(F("AT+CIPSTART=\"UDP\",\"192.168.0.12\",123,123,2\r\n"));
	_esp8266_waitFor("OK\r\n");

	bNTPRunning = true;
}


void CheckNTP()
{

	if ((Serial3.available() > 0)&&(bNTPRunning))
	{
		comdata = Serial3.read();
		Serial.write(comdata);

		if(TimeRequest(comdata)&&bGPS_Valid)
		{
			time_t t1900 = t+0x83aa7e80;
			unsigned char * pT = (unsigned char *)&t1900;
			WIFI_SERIAL.print(F("AT+CIPSEND="));
			WIFI_SERIAL.print(48);
			WIFI_SERIAL.print(F("\r\n"));

			_esp8266_waitFor("OK\r\n>");

			for(unsigned char i = 0; i<40 ; i++)
			{
				WIFI_SERIAL.write(0);
			}
			for(unsigned char i = 0; i<4 ; i++)
			{
				WIFI_SERIAL.write(*(pT+3-i));
			}
			for(unsigned char i = 0; i<4 ; i++)
			{
				WIFI_SERIAL.write(0);
			}
		}
	}
}

bool GetAir()
{

	WIFI_SERIAL.print(F("AT+CIPSTART=\"SSL\",\"free-api.heweather.com\",443\r\n"));
	_esp8266_waitFor("OK\r\n");


	WIFI_SERIAL.print(F("AT+CIPSEND="));
	WIFI_SERIAL.print(sizeof(HttpAir));
	WIFI_SERIAL.print(F("\r\n"));

	_esp8266_waitFor("OK\r\n>");

	for(unsigned char i = 0; i<sizeof(HttpAir) ; i++)
	{
		WIFI_SERIAL.print(HttpAir[i]);
	}

	_esp8266_waitFor("200 OK");




	//_esp8266_getValue("text\":\"",WeatherNow,&InfoLen,0,7); 
	//_esp8266_getValue("pm25\":\"",WeatherNow+7,&InfoLen,0,3); 


	_esp8266_waitFor("CLOSED\r\n");


}
bool GetNow()
{
	WIFI_SERIAL.print(F("AT+CIPSTART=\"SSL\",\"api.seniverse.com\",443\r\n"));
	_esp8266_waitFor("OK\r\n");


	WIFI_SERIAL.print(F("AT+CIPSEND="));
	WIFI_SERIAL.print(sizeof(HttpNow));
	WIFI_SERIAL.print(F("\r\n"));

	_esp8266_waitFor("OK\r\n>");

	for(unsigned char i = 0; i<sizeof(HttpNow) ; i++)
	{
		WIFI_SERIAL.print(HttpNow[i]);
	}

	_esp8266_waitFor("200 OK");




	//_esp8266_getValue("text\":\"",DateLine2,&InfoLen,0,7); 
	_esp8266_getValue("ture\":\"",DateLine2,&InfoLen,0,2); 


	_esp8266_waitFor("CLOSED\r\n");

}
bool GetForcast(unsigned char Day,unsigned char Postion)
{

	HttpForcast[90]=Day+0x30;


	WIFI_SERIAL.print(F("AT+CIPSTART=\"SSL\",\"api.seniverse.com\",443\r\n"));
	_esp8266_waitFor("OK\r\n");

	WIFI_SERIAL.print(F("AT+CIPSEND="));
	WIFI_SERIAL.print(sizeof(HttpForcast));
	WIFI_SERIAL.print(F("\r\n"));

	_esp8266_waitFor("OK\r\n>");

	for(unsigned char i = 0; i<sizeof(HttpForcast) ; i++)
	{
		WIFI_SERIAL.print(HttpForcast[i]);
	}

	_esp8266_waitFor("200 OK");

	_esp8266_getValue("date\":\"",&(WeatherDay[Postion][0]),&InfoLen,5,5); 
	_esp8266_getValue("e_day\":\"",&WeatherCode[Postion][0],&InfoLen,0,2); 
	_esp8266_getValue("high\":\"",&Weather[Postion][3],&InfoLen,0,2); 
	_esp8266_getValue("low\":\"",&Weather[Postion][0],&InfoLen,0,3); 

	//Weather[Postion][2] = '~';

	_esp8266_waitFor("CLOSED\r\n");

}



void HourlyUpdate()
{

	bNTPRunning = false;

	memset(WeatherDay,32,20);
	memset(Weather,32,20);

	memset(DateLine,32,10);
	memset(DateLine2,32,10);
	memset(WeatherCode,0,3);



	WIFI_SERIAL.print(F("AT+CIPCLOSE\r\n"));
	_esp8266_waitFor("OK\r\n");



	GetNow();

	//GetAir();

	if (hour(t+8*3600)<18)
	{
		GetForcast(0,0);
		GetForcast(1,1);
	} 
	else
	{
		GetForcast(1,0);
		GetForcast(2,1);
	}





	StartNTP();

	unsigned char tempDay = day(t+8*3600);
	unsigned char tempMonth = month(t+8*3600);
	unsigned int tempYearh = year(t+8*3600);

	unsigned char week = weekday(t+8*3600);
	DateLine2[7] = WeekStr[(week-1)*3];
	DateLine2[8] = WeekStr[(week-1)*3+1];
	DateLine2[9] = WeekStr[(week-1)*3+2];


	DateLine[0]=tempYearh/1000%10+0x30;
	DateLine[1]=tempYearh/100%10+0x30;
	DateLine[2]=tempYearh/10%10+0x30;
	DateLine[3]=tempYearh%10+0x30;

	DateLine[4] = '-';

	DateLine[5]=tempMonth/10%10+0x30;
	DateLine[6]=tempMonth%10+0x30;

	DateLine[7] = '-';

	DateLine[8]=tempDay/10%10+0x30;
	DateLine[9]=tempDay%10+0x30;


	//左上角  置0
	for (int j=8; j<16; j++)
	{
		for (int i=0; i<16; i++)
		{
			DispBuf[j][i] = 0;
		}
	}
	//日期 

	for (int j=8*8; j<64+60; j++)
	{
		for (int i=0; i<8; i++)
		{
			DispBuf[j/8][i] |= (
				(
				((j-64+1)%6==0)
				?
				0
				:
			((ACSII57[DateLine[(j-64)/6]-32][(j-64)%6]>>i)&1)
				) 
				<<(j%8)
				);

		}

	}
	//当前天气
	for (int j=8*8; j<64+60; j++)
	{
		for (int i=8; i<16; i++)
		{
			DispBuf[j/8][i] |= (
				(
				((j-64+1)%6==0)
				?
				0
				:
			((ACSII57[DateLine2[(j-64)/6]-32][(j-64)%6]>>(i-8))&1)
				)
				<<(j%8)
				);

		}
	}


	//左下角  置0
	for (int j=8; j<16; j++)
	{
		for (int i=0+16; i<16+16; i++)
		{
			DispBuf[j][i] = 0;
		}
	}
	//第一天 日期、气温

	for (int j=8*8; j<64+30; j++)
	{
		for (int i=0+16; i<8+16; i++)
		{
			DispBuf[j/8][i] |= (
				(
				((j-64+1)%6==0)
				?
				0
				:
			((ACSII57[WeatherDay[0][(j-64)/6]-32][(j-64)%6]>>(i-16))&1)
				) 
				<<(j%8)
				);
		}
	}
	//第一天 天气
	for (int j=8*8; j<64+30; j++)
	{
		for (int i=8+16; i<16+16; i++)
		{
			DispBuf[j/8][i] |= (
				(
				((j-64+1)%6==0)
				?
				0
				:
			((ACSII57[Weather[0][(j-64)/6]-32][(j-64)%6]>>(i-8-16))&1)
				)
				<<(j%8)
				);

		}
	}


	//右下角  置0
	for (int j=0; j<8; j++)
	{
		for (int i=0+16; i<16+16; i++)
		{
			DispBuf[j][i] = 0;
		}
	}
	//第二天 日期、气温

	for (int j=0; j<30; j++)
	{
		for (int i=0+16; i<8+16; i++)
		{
			DispBuf[j/8][i] |= (
				(
				((j+1)%6==0)
				?
				0
				:
			((ACSII57[WeatherDay[1][(j)/6]-32][(j)%6]>>(i-16))&1)
				) 
				<<(j%8)
				);

		}

	}
	//第二天 天气
	for (int j=0; j<30; j++)
	{
		for (int i=8+16; i<16+16; i++)
		{
			DispBuf[j/8][i] |= (
				(
				((j+1)%6==0)
				?
				0
				:
			((ACSII57[Weather[1][(j)/6]-32][(j)%6]>>(i-8-16))&1)
				)
				<<(j%8)
				);

		}
	}

	//printf( "WeatherCode = %d ; %s \r\n", atoi((const char*)WeatherCode[0]),atoi((const char*)WeatherCode[1]));




	unsigned char ChartIndex[2];


	//WeatherCode2ChartIndex(20,ChartIndex);
	WeatherCode2ChartIndex(atoi((const char*)WeatherCode[0]),ChartIndex);

	ShowChart(ChartIndex[0],4+8,16);
	ShowChart(ChartIndex[1],6+8,16);

	//WeatherCode2ChartIndex(28,ChartIndex);
	WeatherCode2ChartIndex(atoi((const char*)WeatherCode[1]),ChartIndex);

	ShowChart(ChartIndex[0],4,16);
	ShowChart(ChartIndex[1],6,16);


}

void WeatherCode2ChartIndex(unsigned char Code,unsigned char * ChartIndex)
{
	unsigned char TempChat[2];
	switch(Code)
	{
	case 0://	晴
	case 1://晴
	case 2://晴
	case 3://晴
		ChartIndex[1] = KONG;
		ChartIndex[0] = QING;

		break;

	case 4://多云
	case 5://多云	Partly Cloudy	晴间多云
	case 6://多云	Partly Cloudy	晴间多云
	case 7://多云	Mostly Cloudy	大部多云
	case 8://多云	Mostly Cloudy	大部多云
		ChartIndex[0] = DUO;
		ChartIndex[1] = YUN;
		break;

	case 9://阴	Overcast	阴
		ChartIndex[1] = KONG;
		ChartIndex[0] = YING;
		break;

	case 10://阵雨	Shower	阵雨
		ChartIndex[0] = ZHEN;
		ChartIndex[1] = YU;
		break;

	case 11://雷雨
	case 12://雷雨
		ChartIndex[0] = LEI;
		ChartIndex[1] = YU;
		break;

	case 13://小雨	Light Rain	小雨
		ChartIndex[0] = XIAO;
		ChartIndex[1] = YU;
		break;

	case 14://中雨	Moderate Rain	中雨
		ChartIndex[0] = ZHONG;
		ChartIndex[1] = YU;
		break;

	case 15://大雨	Heavy Rain	大雨
		ChartIndex[0] = DA;
		ChartIndex[1] = YU;
		break;



	case 16://暴雨	Storm	暴雨
	case 17://暴雨	Heavy Storm	大暴雨
	case 18://暴雨	Severe Storm	特大暴雨
		ChartIndex[0] = BAO;
		ChartIndex[1] = YU;
		break;

	case 19://冻雨	Ice Rain	冻雨
		ChartIndex[0] = DONG;
		ChartIndex[1] = YU;
		break;

	case 20://雨雪	Sleet	雨夹雪
		ChartIndex[0] = YU;
		ChartIndex[1] = XUE;
		break;

	case 21://阵雪	Snow Flurry	阵雪
		ChartIndex[0] = ZHEN;
		ChartIndex[1] = XUE;
		break;

	case 22://小雪	Light Snow	小雪
		ChartIndex[0] = XIAO;
		ChartIndex[1] = XUE;
		break;

	case 23://中雪	Moderate Snow	中雪
		ChartIndex[0] = ZHONG;
		ChartIndex[1] = XUE;
		break;

	case 24://大雪	Heavy Snow	大雪
		ChartIndex[0] = DA;
		ChartIndex[1] = XUE;
		break;

	case 25://	暴雪	Snowstorm	暴雪
		ChartIndex[0] = BAO;
		ChartIndex[1] = XUE;
		break;

	case 26://浮尘	Dust	浮尘
		ChartIndex[0] = FU;
		ChartIndex[1] = CHENG;
		break;

	case 27://扬沙	Sand	扬沙
		ChartIndex[0] = YANG;
		ChartIndex[1] = SHA;
		break;

	case 28://尘暴	Duststorm	沙尘暴
	case 29://尘暴	Sandstorm	强沙尘暴

		ChartIndex[0] = CHENG;
		ChartIndex[1] = BAO;
		break;

	case 30://雾	Foggy	雾
		ChartIndex[1] = KONG;
		ChartIndex[0] = WU;
		break;

	case 31://霾	Haze	霾
		ChartIndex[1] = KONG;
		ChartIndex[0] = MAI;
		break;


	case 32://风	Windy	风
		ChartIndex[1] = KONG;
		ChartIndex[0] = FENG;
		break;


	case 33://大风	Blustery	大风
		ChartIndex[0] = DA;
		ChartIndex[1] = FENG;
		break;


	case 34://	飓风	Hurricane	飓风
		ChartIndex[0] = JU;
		ChartIndex[1] = FENG;
		break;


	case 35://		风暴	Tropical Storm	热带风暴
		ChartIndex[0] = FENG;
		ChartIndex[1] = BAO;
		break;
;

	case 36://	龙卷	Tornado	龙卷风
		ChartIndex[0] = LONG;
		ChartIndex[1] = JUAN;
		break;



	case 37://	冷	Cold	冷
		ChartIndex[1] = KONG;
		ChartIndex[0] = LENG;
		break;


	case 38://	热	Hot	热
		ChartIndex[1] = KONG;
		ChartIndex[0] = RE;
		break;


	case 99://	未知	Unknown
	default:

		ChartIndex[0] = WEI;
		ChartIndex[1] = ZHI;
		break;



	}
}

void ShowChart(unsigned char ChartIndex,unsigned char PostionX,unsigned char PostionY)
{

	for(unsigned char y=0; y<16; y++)
	{
		for (unsigned char i=0; i<8; i++)
		{
			DispBuf[PostionX][y+PostionY] = DispBuf[PostionX][y+PostionY]<<1;
			DispBuf[PostionX][y+PostionY]|=(Chart[32*ChartIndex+y*2]>>i)&1;


			DispBuf[PostionX+1][y+PostionY] = DispBuf[PostionX+1][y+PostionY]<<1;
			DispBuf[PostionX+1][y+PostionY]|=(Chart[32*ChartIndex+y*2+1]>>i)&1;


		}
	}
}


void hc595senddata(byte data,byte data2,byte data3,byte data4)//发送上下半屏，各一行数据。
{
	byte temp = 0;

	for (byte i=0; i<8;i++) {  
		digitalWrite(CLK,0);  


		temp = 0;
		temp = 
			data&1 //右上
			|
			((data2&1)<<1)
			|
			((data3&1)<<2)
			|
			((data4&1)<<3)
			;


		PORTA = temp;

		data=data>>1;
		data2=data2>>1;
		data3=data3>>1;
		data4=data4>>1;
		digitalWrite(CLK,1); 
	}  
	//DataUpdate();

}

void Hdot()
{
	//字库取模改成横向
	unsigned char pix = 0;
	for (int k=0; k<96; k++)
	{
		for (int i=0; i<16; i++)//暂存
		{
			ASCII816Buf[i]=ASCII816[k][i];
		}

		for(int j=0; j<8; j++)//转换上半部分8*8
		{
			pix = 0;
			for(int i=0; i<8; i++)
			{
				pix = pix<<1;
				pix=pix+((ASCII816Buf[7-i]>>j)&1);
			}
			ASCII816[k][j] = pix;
		}

		for(int j=8; j<16; j++)//转换下半部分8*8
		{
			pix = 0;
			for(int i=8; i<16; i++)
			{
				pix = pix<<1;
				pix=pix+((ASCII816Buf[16+7-i]>>(j-8))&1);
			}
			ASCII816[k][j] = pix;
		}
	}
}



void hc138sacn(byte r)   //输出行线状态ABCD （A低,D高)
{
	digitalWrite(RowA,(r & 0x01));
	digitalWrite(RowB,(r & 0x02));
	digitalWrite(RowC,(r & 0x04));
	digitalWrite(RowD,(r & 0x08));
}



void DataUpdate()
{
	if (Serial2.available() > 0)
	{
		comdata = Serial2.read();
		//Serial.write(comdata);
		if(GpsUpdate(comdata))
		{

		}
	}


}


bool GpsUpdate(unsigned char k)
{
	static unsigned char SepIndex = 0;
	static unsigned char LastSeconds = 0;
	if (GPRMC_command ==1)
	{
		if (k == ',')
		{
			SepIndex++;
			ElementIndex = 0;
			return false;
		}

		switch(SepIndex)
		{
		case 1:
			if (ElementIndex < 6)
			{
				GPS_Time[ElementIndex] = k;
			}
			ElementIndex ++;
			break;

		case 2:
			GPS_Valid[ElementIndex] = k;
			ElementIndex ++;
			break;


		case 9:

			GPS_Date[ElementIndex] = k;
			ElementIndex ++;

			break;
		}

		if (SepIndex > 11)
		{
			GPRMC_Index = 0	;
			GPRMC_command = 0;
			SepIndex = 0;



			if (LastSeconds!=GPS_Time[5])
			{
				LastSeconds=GPS_Time[5];
				OnSeconds();
				return true;
			} 
			else
			{
				return false;
			}



		}
	}
	else
	{
		if (GPRMC_command_array[GPRMC_Index] == k)
		{
			GPRMC_Index++;
			if (GPRMC_Index>5)
			{
				GPRMC_command =1;
			}
		}
		else
		{
			GPRMC_Index=0;
		}
	}
	return false;
}

void OnSeconds()
{

	SecondsSinceStart++;

	//printf("SecondsSinceStart = %d\r\n",SecondsSinceStart);


	//Update time to UTC
	tm.Hour = (GPS_Time[0]-0x30)*10+(GPS_Time[1]-0x30);
	tm.Minute = (GPS_Time[2]-0x30)*10+(GPS_Time[3]-0x30);
	tm.Second = (GPS_Time[4]-0x30)*10+(GPS_Time[5]-0x30);
	tm.Day = (GPS_Date[0]-0x30)*10+(GPS_Date[1]-0x30);
	tm.Month = (GPS_Date[2]-0x30)*10+(GPS_Date[3]-0x30);
	tm.Year = (GPS_Date[4]-0x30)*10+(GPS_Date[5]-0x30)+30;
	t = makeTime(tm);

	unsigned char Hour = tm.Hour + 8;
	if (Hour>23)
	{
		Hour = Hour - 24;
	}



	TimeLine[0]=Hour/10%10+0x30;
	TimeLine[1]=Hour%10+0x30;
	TimeLine[3]=GPS_Time[2];
	TimeLine[4]=GPS_Time[3];
	TimeLine[6]=GPS_Time[4];
	TimeLine[7]=GPS_Time[5];

	//if ((GPS_Time[4] == '0')&&(GPS_Time[5] == '0')) //(GPS_Time[2] == '0')&&(GPS_Time[3] == '0')&&
	if ((GPS_Time[2] == '0')&&(GPS_Time[3] == '0')&&(GPS_Time[4] == '0')&&(GPS_Time[5] == '0')) //
	{
		HourlyUpdate();
	}

	//信号接收状态
	if (GPS_Valid[0]=='A')
	{
		bGPS_Valid = true;
		TimeLine[2] = ':';
		TimeLine[5] = ':';
	}
	else
	{
		bGPS_Valid = false;
		TimeLine[2] = ':';
		TimeLine[5] = '.';
	}





	//字符串->显示缓存

	//时间
	for (int j=0; j<8; j++)
	{
		for (int i=0; i<16; i++)
		{
			DispBuf[j][i] = ASCII816[TimeLine[j]-32][i];

		}
	}


}

bool TimeRequest(unsigned char k)
{
	static unsigned char TimeRequest_Index = 0;

	if (Time_request_array[TimeRequest_Index] == k)
	{
		TimeRequest_Index++;
		if (TimeRequest_Index>9)
		{
			TimeRequest_Index=0;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
		TimeRequest_Index=0;
	}
}

bool _esp8266_waitFor(const char *string) {
	char so_far = 0;
	char received;
	int counter = 0;
	do {
		//received = _esp8266_getch();
		if (!_esp8266_getch(&received))
		{
			return false;
		}
		counter++;
		if (received == string[so_far]) {
			so_far++;
		} else {
			so_far = 0;
		}
	} while (string[so_far] != 0);
	return true;
}

bool _esp8266_getValue(const char *string,unsigned char  * Value,unsigned char * Len,unsigned char Start,unsigned char Max) 
{

	char received;
	*Len = 0;
	unsigned char CurrentChar = 0;

	if(!_esp8266_waitFor(string)) return false;

	unsigned long RecvStartTime = SecondsSinceStart;
	while(1) 
	{
		if (!_esp8266_getch(&received))
		{
			return false;
		}

		if ((received == '"')||((*Len)>=Max))
		{
			return true;
		} 
		else
		{
			if (CurrentChar >= Start)
			{
				*(Value+(*Len)) = received;
				(*Len) = (*Len)+1;
			} 
			else
			{
				CurrentChar++;
			}

		}

		if (SecondsSinceStart - RecvStartTime > TIME_OUT)
		{
			return false;
		}
	} 

}




//**Function to get one byte of date from UART**//
bool _esp8266_getch(char * RetData)   
{
	unsigned long RecvStartTime = SecondsSinceStart;
	while (1)
	{


		if (WIFI_SERIAL.available() > 0)
		{
			*RetData = WIFI_SERIAL.read();
			Serial.write(*RetData);
			return true;
		}
		else
		{
			NonStopTask();
			DataUpdate();
		}
		if (SecondsSinceStart - RecvStartTime > TIME_OUT)
		{
			return false;
		}
	}
}


