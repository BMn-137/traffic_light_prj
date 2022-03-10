#include  <avr/io.h>
#include  <util/delay.h>
#define   delay_ms(x)   _delay_ms(x)
#include  <avr/interrupt.h>

const unsigned char disp[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
unsigned char ledbuf[]={0x00,0x00,0x00,0x00};// ��ʾ���������ֱ��ŵ����ϱ��Ͷ��������ʮλ����λ����

int ms10=0,sec=0,x1,x2;


int a1=10,b1=6,c1=4,a2=10,b2=7,c2=3;
int nbred,nbgreen,nbyellow,dxred,dxgreen,dxyellow;  


int x=0,dx=-1,nb=-1,shanshuo=1,k=-1,turn=-1,save=0,STOP_music=0,T=-1,CLOCK=0,led=0,sos=-1;










/******��ʱ��1�ĳ�ʼ����CTCģʽ��8��Ƶ���ж�����5ms******/
void  disp_init(void)
{
	OCR1A = 4999;		//100Hz=8MHz/(2*8*(1+OCR1A))
	TCCR1A = 0x00;
	TCCR1B = (1 << WGM12);        //CTCģʽ
	TCCR1B |= (1 << CS11);	//8��Ƶ
	TIMSK |= (1 << OCIE1A);	//���Ƚ�ƥ���ж�A
}




/******�������ʾ���� ******/
void display(char num,char pos)
{ 	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);
	PORTB &= 0x0F; 		//��λѡ
	PORTB &=~(1<<0);
	SPDR=num;
	while(0==(SPSR&0X80)); 	//�ȴ� SPIF�жϽ�����λ
	PORTB|=(1<<0);
	PORTB |= 1<<(7-pos);
}






/********����ܿ���IO��ʼ��*******/
void io_init(void)

{
	PORTC = 0x00;   //��
	DDRC  = 0xFF;	//��
	
	
	DDRD = 0X00;	//��λ��������
	PORTD = 0XFF;	//PD��8�������˿����룬����
	
	DDRE=0xFF;
	PORTE&=~(1<<1); //74HC595ʹ��
	
	PORTB = 0xFF;	//PB4\5\6\7���������λ��
	DDRB |= 0xFF;   //(1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4);
}
	






//ɨ����̣���ü���
void key_read(void)
{
	unsigned char i,j; 	 	 	 	 	 	//�����¼
	unsigned char key_num;
	i=PIND; 	 	 	 					//������ʾ������ ����¼
	delay_ms(20);  	 	 					//ȥ��������
	j = PIND; 								//j=������İ������
	if(i == j) 	 	 	 	//���ζԱ�ȷ����������,���а�������
	{
		switch (i) 							 //��������ת���ɼ�ֵ
		{
			case  0x7F: 	save=(++save)%2;      break;    //8  ��ʱ�������ɺ�Ĵ浵 ����д��EEPROM 
			case  0xBF:   CLOCK=(++CLOCK)%3;
			STOP_music=0;  		break;    //7  
			case  0xDF:  sos=(++sos)%3; 	break;	  	  //6  ����ģʽ 0��ֹͨ��    1�ϱ�ͨ��      2����ͨ�� 
		
			
								 
			case  0xEF: 	x=(++x)%3;		break;	  	  //5  ģʽ�л� 0Ϊ��ͨģʽ  1Ϊҹ��ģʽ    2Ϊ����ģʽ 
			
			
			
			case  0xF7: 	dx=(++dx)%5;	break;	   	  //4  ʱ�����,0������� 1�����̵� 2�����Ƶ�   3Ц��ȷ������
			case  0xFB:   nb=(++nb)%5;      break;   	//3  ʱ�����,0�ϱ���� 1�ϱ��̵� 2�ϱ��Ƶ�    3Ц��ȷ������
		
		
			case  0xFE: 								//1
			if(nb==0) a1--;       	//�ϱ��ĺ���̵�  ��һ����Ӧ��һ      
			if(nb==1) b1--;
			if(nb==2) c1--;
			
			if(dx==0) a2--;       	//�����ĺ���̵�  ��һ����Ӧ��һ      
			if(dx==1) b2--;
			if(dx==2) c2--;
			
			break;
			
			
			
			
			case  0xFD: 			//2 
			if(nb==0) a1++;		  //�ϱ��ĺ���̵�  �ڶ�����Ӧ��һ
			if(nb==1) b1++;
			if(nb==2) c1++;
		
		
			if(dx==0) a2++;		  //�����ĺ���̵�  �ڶ�����Ӧ��һ
			if(dx==1) b2++;
			if(dx==2) c2++;
			break;
		
		
		
		
			default:      break;
		}
			while(PIND!=0xFF)  ; 				//�ȴ������ɿ�
	}
}



/******�жϷ������Ĺ���******/
ISR(TIMER1_COMPA_vect)

{
		static unsigned char j=0,c=0; //��ʾˢ�±�־
	j++;c++;
	k=(++k)%4; 				     	//k ʵ������ˢ�������


    


	if(j>99)  {shanshuo ^= 1;j=0;}  //С������˸��������   1s=0.05*100*2
	
	
	
	
	
	/*************��ͨ��ʱ��д����**************/
	if(c>199)
	{
		c=0;
		if(save==1)      //����ͨ��ʱ������д��EEPROM
		{
			
			
			
			
		    EEPROM_write(100,a1);
		    EEPROM_write(101,b1);
		    EEPROM_write(102,c1);
		    EEPROM_write(103,a2);
		    EEPROM_write(104,b2);
		    EEPROM_write(105,c2);
		}	
		
	}
	
	
	
		/*******ʱ�������Χ�����ֱ任*******/
	if(ms10>99)   
	{
	    ms10=0;
	    
		  
		
		x1=nbred;nbred--;
		x2=dxgreen;dxgreen--;  //100 * 10 =1s
		PORTC = 0x0A;
	   
	
	    if(nbred<0)     
		{
		    x1=nbgreen;nbgreen--;//�ϱ����̻Ʊ任 
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
		    x2=dxyellow;dxyellow--;//�����̻ƺ�任 
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
	
	
	
	
	
	
	
	
	
	
		/******ʱ�������ʾ*******/
	if(nb==0)     //�ϱ���Ƶ���
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

	if(nb==1)   //�ϱ��̵Ƶ���
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

	if(nb==2)		//�ϱ��ƵƵ���
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
	
	if(nb==3)		//����ȷ�� 
	{
	    dx=-1;x=-1;
		PORTC = 0x00;
		ledbuf[1] = 0x23;
		
		ledbuf[0] = 0x23;
	    ledbuf[2] = 0x23;
	    ledbuf[3] = 0x23;
		display(ledbuf[k],k);
	}
	
	
	
	
	
	
	
	if(dx==0)     //������Ƶ���
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

	if(dx==1)   //�����̵Ƶ���
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

	if(dx==2)		//�����ƵƵ���
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

	if(dx==3)		//����ȷ�� 
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
	
	
	
	
	
	
	display(ledbuf[k],k);      //�������ʾ����
}






/************AD���ѹֵʵ�ֹ�ؿ���ҹ��ģʽ************/
unsigned int get_ad(void) 
{ 
   long int i;  
   ADMUX = (1 << REFS0);  //�ο���ѹAVCC
   ADCSRA = (1<< ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); //ADCʹ�ܣ���ʼת����8��Ƶ 
   while(!(ADCSRA & (1 << ADIF))); //�ȴ�ת�������жϱ�־��λ 
   i = ADC; //ȡת�����
   ADCSRA &= ~(1 << ADIF); //����жϱ�־ 
   ADCSRA &= ~(1 << ADEN); //�ر�ADC 
   return i; 
}
 



	
	
	
	
	
	
/************EEPROMд����************/

void EEPROM_write(unsigned int Address,unsigned char Data)
{
	while(EECR&(1<<EEWE));			//�ȴ���һ��д��������
	EEAR = Address;  				//��ַ
	EEDR = Data;  					//����
	EECR |= (1<<EEMWE);  			//��λEEMWE
	EECR |= (1<<EEWE);				//��λEEWE����д����
}





/************EEPROM������************/
int EEPROM_read(unsigned int Address)
{
	while(EECR&(1<<EEWE));			//�ȴ���һ��д��������
	EEAR = Address;  				//��ַ
	EECR |= (1<<EERE); 				//��λEEMWE
	return EEDR;  					//���ض�ȡ���
}





//�� �� �� 
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
	
	
	

   
	
	
		
	





	
	
	
    	/********�����ý�ͨ��ʱ��*********/

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
        
		key_read(); 	 	//����ɨ��
		delay_ms(50); 		//����ɨ����
		
		
        i = get_ad()/204.8;


		
		if(x==0)//��ͨģʽ 
		{
			T=-1;sos=-1;			//������������ʱ����ʾ����
	
	      if(i>2)                //��ش����ĵ�ѹֵ��Ϊ�Ƿ�Ϊҹ��ģʽ��׼
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
		
		
		if(x==1)//ҹ��ģʽ 
		{

            T=-1;sos=-1;			//������������ʱ����ʾ����
            ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;

			if(shanshuo==1)
			PORTC = 0x24;
			else
			PORTC = 0x00;
			
		} 
		
		
		
		if(x==2)//����ģʽ 
		{
			nb=-1;dx=-1;T=-1;;		//������������ʱ����ʾ����
			ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;
			PORTC = 0x09;
			if(sos==0) PORTC = 0x09;
			if(sos==1) PORTC = 0x0A;
			if(sos==2) PORTC = 0x11;
			
			
		} 
			
	 
	    

    

	      
	
	
	    
			
	
		delay_ms(10); //����ɨ����
	}
	
	
	
} 
	
	
	
	
	
	

	
	
	
	
	
	
