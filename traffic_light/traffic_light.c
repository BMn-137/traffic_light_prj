#include  <avr/io.h>
#include  <util/delay.h>
#define   delay_ms(x)   _delay_ms(x)
#include  <avr/interrupt.h>

const unsigned char disp[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
unsigned char ledbuf[]={0x00,0x00,0x00,0x00};// 显示缓冲区，分别存放的是南北和东西方向的十位、个位段码

int ms10=0,sec=0,x1,x2;


int a1=10,b1=6,c1=4,a2=10,b2=7,c2=3;
int nbred,nbgreen,nbyellow,dxred,dxgreen,dxyellow;  


int x=0,dx=-1,nb=-1,shanshuo=1,k=-1,turn=-1,save=0,STOP_music=0,T=-1,CLOCK=0,led=0,sos=-1;










/******定时器1的初始化，CTC模式，8分频，中断周期5ms******/
void  disp_init(void)
{
	OCR1A = 4999;		//100Hz=8MHz/(2*8*(1+OCR1A))
	TCCR1A = 0x00;
	TCCR1B = (1 << WGM12);        //CTC模式
	TCCR1B |= (1 << CS11);	//8分频
	TIMSK |= (1 << OCIE1A);	//开比较匹配中断A
}




/******数码管显示函数 ******/
void display(char num,char pos)
{ 	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);
	PORTB &= 0x0F; 		//关位选
	PORTB &=~(1<<0);
	SPDR=num;
	while(0==(SPSR&0X80)); 	//等待 SPIF中断结束置位
	PORTB|=(1<<0);
	PORTB |= 1<<(7-pos);
}






/********数码管控制IO初始化*******/
void io_init(void)

{
	PORTC = 0x00;   //灯
	DDRC  = 0xFF;	//灯
	
	
	DDRD = 0X00;	//八位独立按键
	PORTD = 0XFF;	//PD口8个按键端口输入，上拉
	
	DDRE=0xFF;
	PORTE&=~(1<<1); //74HC595使能
	
	PORTB = 0xFF;	//PB4\5\6\7控制数码管位码
	DDRB |= 0xFF;   //(1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4);
}
	






//扫描键盘，获得键码
void key_read(void)
{
	unsigned char i,j; 	 	 	 	 	 	//键码记录
	unsigned char key_num;
	i=PIND; 	 	 	 					//按键表示的数字 并记录
	delay_ms(20);  	 	 					//去按键颤抖
	j = PIND; 								//j=除抖后的按键情况
	if(i == j) 	 	 	 	//二次对比确定按键操作,且有按键按下
	{
		switch (i) 							 //将按键码转换成键值
		{
			case  0x7F: 	save=(++save)%2;      break;    //8  灯时间调整完成后的存档 数据写入EEPROM 
			case  0xBF:   CLOCK=(++CLOCK)%3;
			STOP_music=0;  		break;    //7  
			case  0xDF:  sos=(++sos)%3; 	break;	  	  //6  紧急模式 0禁止通行    1南北通行      2东西通行 
		
			
								 
			case  0xEF: 	x=(++x)%3;		break;	  	  //5  模式切换 0为普通模式  1为夜间模式    2为紧急模式 
			
			
			
			case  0xF7: 	dx=(++dx)%5;	break;	   	  //4  时间调整,0东西红灯 1东西绿灯 2东西黄灯   3笑脸确认设置
			case  0xFB:   nb=(++nb)%5;      break;   	//3  时间调整,0南北红灯 1南北绿灯 2南北黄灯    3笑脸确认设置
		
		
			case  0xFE: 								//1
			if(nb==0) a1--;       	//南北的红黄绿灯  第一键对应减一      
			if(nb==1) b1--;
			if(nb==2) c1--;
			
			if(dx==0) a2--;       	//东西的红黄绿灯  第一键对应减一      
			if(dx==1) b2--;
			if(dx==2) c2--;
			
			break;
			
			
			
			
			case  0xFD: 			//2 
			if(nb==0) a1++;		  //南北的红黄绿灯  第二键对应加一
			if(nb==1) b1++;
			if(nb==2) c1++;
		
		
			if(dx==0) a2++;		  //东西的红黄绿灯  第二键对应加一
			if(dx==1) b2++;
			if(dx==2) c2++;
			break;
		
		
		
		
			default:      break;
		}
			while(PIND!=0xFF)  ; 				//等待按键松开
	}
}



/******中断服务程序的功能******/
ISR(TIMER1_COMPA_vect)

{
		static unsigned char j=0,c=0; //显示刷新标志
	j++;c++;
	k=(++k)%4; 				     	//k 实现轮流刷新数码管


    


	if(j>99)  {shanshuo ^= 1;j=0;}  //小数点闪烁周期设置   1s=0.05*100*2
	
	
	
	
	
	/*************交通灯时间写操作**************/
	if(c>199)
	{
		c=0;
		if(save==1)      //将交通灯时间数据写入EEPROM
		{
			
			
			
			
		    EEPROM_write(100,a1);
		    EEPROM_write(101,b1);
		    EEPROM_write(102,c1);
		    EEPROM_write(103,a2);
		    EEPROM_write(104,b2);
		    EEPROM_write(105,c2);
		}	
		
	}
	
	
	
		/*******时间调整范围及数字变换*******/
	if(ms10>99)   
	{
	    ms10=0;
	    
		  
		
		x1=nbred;nbred--;
		x2=dxgreen;dxgreen--;  //100 * 10 =1s
		PORTC = 0x0A;
	   
	
	    if(nbred<0)     
		{
		    x1=nbgreen;nbgreen--;//南北红绿黄变换 
		    PORTC = PORTC + 0x08;
		    
     	    if(nbgreen<0)      
			{
			    x1=nbyellow;nbyellow--;
			    PORTC = PORTC + 0x10;
			    
	            if(nbyellow<0)      
			    {nbred=a1;nbgreen=b1;nbyellow=c1;x1=nbred;PORTC = PORTC - 0x18;}
			}
	    }
	
	 
	    if(dxgreen<0)      
		{
		    x2=dxyellow;dxyellow--;//东西绿黄红变换 
		    PORTC = PORTC + 0x02;
		    
	        if(dxyellow<0)      
			{
			    x2=dxred;dxred--;
			    PORTC = PORTC - 0x03;
			    
	            if(dxred<0)          
			    {dxgreen=b2;dxred=a2;dxyellow=c2;x2=dxgreen;PORTC = PORTC + 0x01;}
			}
	    }
	    
	    
	}	
	
	
	
	
	
	
	
	
	
	
		/******时间调整显示*******/
	if(nb==0)     //南北红灯调整
	{
        

	    dx=-1;x=-1;
		PORTC = 0x08;
		if(shanshuo==1)
		ledbuf[1] = disp[a1%10];
		else
		ledbuf[1] = 0x00;
		
		ledbuf[0] = disp[a1/10];
		ledbuf[2] = 0x00;
		ledbuf[3] = 0x00;
		display(ledbuf[k],k);
	}

	if(nb==1)   //南北绿灯调整
	{

	    dx=-1;x=-1;
		PORTC = 0x10;
		if(shanshuo==1)
		ledbuf[1] = disp[b1%10];
		else
		ledbuf[1] = 0x00;

		ledbuf[0] = disp[b1/10];
		ledbuf[2] = 0x00;
		ledbuf[3] = 0x00;
		display(ledbuf[k],k);
		
	}

	if(nb==2)		//南北黄灯调整
	{
	    dx=-1;x=-1;
		PORTC = 0x20;
		if(shanshuo==1)
		ledbuf[1] = disp[c1%10];
		else
		ledbuf[1] = 0x00;
		
		ledbuf[0] = disp[c1/10];
	    ledbuf[2] = 0x00;
	    ledbuf[3] = 0x00;
		display(ledbuf[k],k);
	}
	
	if(nb==3)		//设置确认 
	{
	    dx=-1;x=-1;
		PORTC = 0x00;
		ledbuf[1] = 0x23;
		
		ledbuf[0] = 0x23;
	    ledbuf[2] = 0x23;
	    ledbuf[3] = 0x23;
		display(ledbuf[k],k);
	}
	
	
	
	
	
	
	
	if(dx==0)     //东西红灯调整
	{
	    nb=-1;x=-1;
		PORTC = 0x01;
		if(shanshuo==1)
		ledbuf[3] = disp[a2%10];
		else
		ledbuf[3] = 0x00;
		
		ledbuf[2] = disp[a2/10];
		ledbuf[1] = 0x00;
		ledbuf[0] = 0x00;
		display(ledbuf[k],k);
	}

	if(dx==1)   //东西绿灯调整
	{
	    nb=-1;x=-1;
		PORTC = 0x02;
		if(shanshuo==1)
		ledbuf[3] = disp[b2%10];
		else
		ledbuf[3] = 0x00;

		ledbuf[2] = disp[b2/10];
		ledbuf[1] = 0x00;
		ledbuf[0] = 0x00;
		display(ledbuf[k],k);
		
	}

	if(dx==2)		//东西黄灯调整
	{
	    nb=-1;x=-1;
		PORTC = 0x04;
		if(shanshuo==1)
		ledbuf[3] = disp[c2%10];
		else
		ledbuf[3] = 0x00;
		
		ledbuf[2] = disp[c2/10];
	    ledbuf[1] = 0x00;
	    ledbuf[0] = 0x00;
		display(ledbuf[k],k);
	}

	if(dx==3)		//设置确认 
	{
	    nb=-1;x=-1;
		PORTC = 0x00;
		ledbuf[1] = 0x23;
		
		ledbuf[0] = 0x23;
	    ledbuf[2] = 0x23;
	    ledbuf[3] = 0x23;
		display(ledbuf[k],k);
	}
	
	
       if(save==1)
	      {  
			nb=-1;dx=-1;sos=-1;
		    PORTC = 0x00;
		    ledbuf[1] = 0x23;
		    ledbuf[0] = 0x23;
	        ledbuf[2] = 0x23;
	        ledbuf[3] = 0x23;
		    display(ledbuf[k],k);
	      }
	
	
	
	
	
	
	display(ledbuf[k],k);      //数码管显示函数
}






/************AD测电压值实现光控开启夜间模式************/
unsigned int get_ad(void) 
{ 
   long int i;  
   ADMUX = (1 << REFS0);  //参考电压AVCC
   ADCSRA = (1<< ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); //ADC使能，开始转换，8分频 
   while(!(ADCSRA & (1 << ADIF))); //等待转换结束中断标志置位 
   i = ADC; //取转换结果
   ADCSRA &= ~(1 << ADIF); //清除中断标志 
   ADCSRA &= ~(1 << ADEN); //关闭ADC 
   return i; 
}
 



	
	
	
	
	
	
/************EEPROM写操作************/

void EEPROM_write(unsigned int Address,unsigned char Data)
{
	while(EECR&(1<<EEWE));			//等待上一次写操作结束
	EEAR = Address;  				//地址
	EEDR = Data;  					//数据
	EECR |= (1<<EEMWE);  			//置位EEMWE
	EECR |= (1<<EEWE);				//置位EEWE启动写操作
}





/************EEPROM读操作************/
int EEPROM_read(unsigned int Address)
{
	while(EECR&(1<<EEWE));			//等待上一次写操作结束
	EEAR = Address;  				//地址
	EECR |= (1<<EERE); 				//置位EEMWE
	return EEDR;  					//返回读取结果
}





//主 函 数 
int main()
{
	ledbuf[3] = 0x00;
	ledbuf[2] = 0x00;
	ledbuf[1] = 0x00;
	ledbuf[0] = 0x00; 


    long int i;
    
    
	io_init();
	disp_init();
	sei();
	
	
	

   
	
	
		
	





	
	
	
    	/********读设置交通灯时间*********/

	if(save==0)
	{
	    T=-1;nb=-1;dx=-1;sos=-1;
		a1=EEPROM_read(100);
		b1=EEPROM_read(101);
		c1=EEPROM_read(102);
		a2=EEPROM_read(103);
		b2=EEPROM_read(104);
		c2=EEPROM_read(105);
	}
	



    nbred=a1,nbgreen=b1,nbyellow=c1;
	dxred=a2,dxgreen=b2,dxyellow=c2;
	


	
	
	while (1)
	{
        
		key_read(); 	 	//键盘扫描
		delay_ms(50); 		//键盘扫描间隔
		
		
        i = get_ad()/204.8;


		
		if(x==0)//普通模式 
		{
			T=-1;sos=-1;			//消除其他功能时间显示干扰
	
	      if(i>2)                //光控处检查的电压值作为是否为夜间模式标准
		  {
		    ms10++;
			ledbuf[3] = disp[x2%10];
			ledbuf[2] = disp[x2/10];
			ledbuf[1] = disp[x1%10];
			ledbuf[0] = disp[x1/10];
	      }
	      
	      else
	      {
	        ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;

			if(shanshuo==1)
			PORTC = 0x24;
			else
			PORTC = 0x00;
		  }
		  
		  
       
	
			
		}
		
		
		if(x==1)//夜间模式 
		{

            T=-1;sos=-1;			//消除其他功能时间显示干扰
            ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;

			if(shanshuo==1)
			PORTC = 0x24;
			else
			PORTC = 0x00;
			
		} 
		
		
		
		if(x==2)//紧急模式 
		{
			nb=-1;dx=-1;T=-1;;		//消除其他功能时间显示干扰
			ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;
			PORTC = 0x09;
			if(sos==0) PORTC = 0x09;
			if(sos==1) PORTC = 0x0A;
			if(sos==2) PORTC = 0x11;
			
			
		} 
			
	 
	    

    

	      
	
	
	    
			
	
		delay_ms(10); //键盘扫描间隔
	}
	
	
	
} 
	
	
	
	
	
	

	
	
	
	
	
	
