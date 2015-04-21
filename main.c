#include <REGX52.H>
#include <intrins.h>

sbit temperature=P1^0; 

#define lcdrs P2_0
#define lcdrw	P2_1
#define lcden	P2_2
#define beep P3_6

unsigned char jtmpvl[19];
unsigned char receive;
bit flag;
int judge;
unsigned char a;
void uart_init();
void uart_send_data(unsigned char Data);
void uart_send_string(unsigned char *s);
void delay(unsigned int z);
void ds18b20_init(void);
void ds18b20_wait(void);
bit ds18b20_read_bit(void);
unsigned char ds18b20_read_byte(void);
void ds18b20_write_byte(unsigned char dat);
void ds18b20_send_change_command(void);
void ds18b20_send_read_command(void);
int ds18b20_get_temperature(void);
void lcd_write_com(unsigned char com);
void lcd_write_data(unsigned char date);
void lcd_init();

void main(){
	int tmpvl;
	receive=0;
	judge = 50;
	uart_init();
	lcd_init();
	delay(50);
	uart_send_string("AT+CIPMUX=1\r\n");
	delay(100);
	uart_send_string("AT+CIPSERVER=1,8081\r\n");
	delay(100);
	uart_send_string("AT+CIPSTO=2880\r\n");
	delay(5000);
	ds18b20_send_change_command();
	while(1){
		tmpvl=ds18b20_get_temperature();
		uart_send_string("AT+CIPSEND=0,2\r\n");
		delay(10);
		uart_send_data(0x30+tmpvl%100/10);
		delay(5);
		uart_send_data(0x30+tmpvl%10);
		delay(1000);
		lcd_write_com(0x87);
		lcd_write_data(0x30+tmpvl%100/10);
		lcd_write_com(0x88);
		lcd_write_data(0x30+tmpvl%10);
		lcd_write_com(0x89);
		lcd_write_data('C');
		ds18b20_send_change_command();
//		lcd_write_com(0x8A);
//		lcd_write_data(jtmpvl[1]);
//		lcd_write_com(0x8B);
//		lcd_write_data(jtmpvl[2]);
		delay(1000);
		ds18b20_send_change_command();
		if(tmpvl > judge)
		{
			beep = 0;
		}
	}
}

void uart_init(){
	PCON &= 0X7F;//0111.1111
	SCON = 0X50;
	TMOD = 0X20;
	TH1 = 0XFD;//9600bps
	TL1 = 0XFD;
	ET1 = 0;
	TR1 = 1;
	REN = 1;
	SM0 = 1;
	SM1 = 1;
	EA = 1;
	ES = 1;
}

void ser() interrupt 4
{
	RI = 0;
	a = SBUF;
	if(a == '$')
	{
//		jtmpvl[1]='1';
//		jtmpvl[2]='2';
		judge = 28;
	}
//	if(flag == 1)
//	{
//		jtmpvl[receive] = a;
//		receive++;
//	}
//	if(receive == 3)
//	{
//		receive = 0;
//		flag = 0;
//	}
}

void uart_send_data(unsigned char Data){
	ES = 0;
	TI = 0;
	SBUF = Data;
	while(TI == 0);
	TI = 0;
	ES = 1;
}

void uart_send_string(unsigned char *s){
	while(*s){
		uart_send_data(*s++);
	}
}

void delay(unsigned int z){
	unsigned int x,y;
	for(x=z;x>0;x--){
		for(y=110;y>0;y--);
	}
}

void ds18b20_init(void){
	unsigned int i;
	temperature=0;
	i=100;
	while(i>0)
		i--;
	temperature=1;
	i=4;
	while(i>0)
		i--;
}
//DS18B20等待函数
void ds18b20_wait(void){
	unsigned int i;
	while(temperature);
	while(~temperature);//IO口电平取反
	i=4;
	while(i>0)
		i--;
}
//DS18B20读取一位数据
bit ds18b20_read_bit(void){
	unsigned int i;
	bit b;
	temperature=0;
	i++;
	temperature=1;
	i++;
	i++;
	b=temperature;
	i=8;
	while(i>0)
		i--;
	return b;
}
//DS18B20读取数据
unsigned char ds18b20_read_byte(void){
	unsigned int i;
	unsigned char j,dat;
	dat=0;
	for(i=0;i<8;i++){
		j=ds18b20_read_bit();
		dat=(j<<7)|(dat>>1);
	}
	return dat;
}
//DS18B20写一位函数
void ds18b20_write_byte(unsigned char dat){
	unsigned int i;
	unsigned char j;
	bit b;
	for(j=0;j<8;j++){
		b=dat&0x01;
		dat>>=1;
		if(b){
			temperature=0;
			temperature=1;
			i=8;
			while(i>0)
				i--;
		}
		else{
			temperature=0;
			i=8;
			while(i>0)
				i--;
			temperature=1;
			i++;
			i++;
		}
	}
}
//DS18B20发送变换命令
void ds18b20_send_change_command(void){
	ds18b20_init();
	ds18b20_wait();
	delay(1);
	ds18b20_write_byte(0xcc);
	ds18b20_write_byte(0x44);
}
//DS18B20发送读取命令
void ds18b20_send_read_command(void){
	ds18b20_init();
	ds18b20_wait();
	delay(1);
	ds18b20_write_byte(0xcc);
	ds18b20_write_byte(0xbe);
}
//DS18B20读取温度
int ds18b20_get_temperature(void){
	unsigned int tmpvalue;
	int value;
	float t;
	unsigned char low,high;
	ds18b20_send_read_command();
	low=ds18b20_read_byte();
	high=ds18b20_read_byte();
	tmpvalue=high;
	tmpvalue<<=8;
	tmpvalue|=low;
	value=tmpvalue;
	t=value*0.0625;
	value=t+(value>0?0.5:-0.5);//判断VALUE的值，如果大于0，则加0.5否则-0.5
	return value;
}

void lcd_write_com(unsigned char com)
{
	lcdrs=0;
	P0=com;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;
}

void lcd_write_data(unsigned char date)
{
	lcdrs=1;
	P0=date;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;
}
void lcd_init()
{
	lcdrs = 0;
	lcdrw = 0;
	lcden=0;
	lcd_write_com(0x38);
	lcd_write_com(0x0e);
	lcd_write_com(0x06);
	lcd_write_com(0x01);
	lcd_write_com(0x80+0x10);
}