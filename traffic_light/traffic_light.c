#include  <avr/io.h>
#include  <util/delay.h>
#define   delay_ms(x)   _delay_ms(x)
#include  <avr/interrupt.h>

const unsigned char disp[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
unsigned char ledbuf[]={0x00,0x00,0x00,0x00};// show_buffer	adding North_Num and South_Num

int ms10=0,sec=0,x1,x2;


int a1=10,b1=6,c1=4,a2=10,b2=7,c2=3;
int nbred,nbgreen,nbyellow,dxred,dxgreen,dxyellow;  


int x=0,dx=-1,nb=-1,shanshuo=1,k=-1,turn=-1,save=0,STOP_music=0,T=-1,CLOCK=0,led=0,sos=-1;










/******init Time1，CTC_model，8 frequency_division，interrupt_cycle 5ms******/
void  disp_init(void)
{
	OCR1A = 4999;		//100Hz=8MHz/(2*8*(1+OCR1A))
	TCCR1A = 0x00;
	TCCR1B = (1 << WGM12);        //CTC_model
	TCCR1B |= (1 << CS11);	//8 frequency_division
	TIMSK |= (1 << OCIE1A);	//start compare_interrupt_A
}




/******LED show ******/
void display(char num,char pos)
{ 	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);
	PORTB &= 0x0F; 		//bit_select off
	PORTB &=~(1<<0);
	SPDR=num;
	while(0==(SPSR&0X80)); 	//wait SPIF interrupt_end_bit
	PORTB|=(1<<0);
	PORTB |= 1<<(7-pos);
}






/********LED IO init*******/
void io_init(void)

{
	PORTC = 0x00;   //led
	DDRC  = 0xFF;	//led
	
	
	DDRD = 0X00;	//8_button
	PORTD = 0XFF;	//PD_pin 8_button input
	
	DDRE=0xFF;
	PORTE&=~(1<<1); //74HC595 init
	
	PORTB = 0xFF;	//PB4\5\6\7_control
	DDRB |= 0xFF;   //(1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4);
}
	






//scan keyboard, get button_value
void key_read(void)
{
	unsigned char i,j; 	 	 	 	 	 	//button_value
	unsigned char key_num;
	i=PIND; 	 	 	 					
	delay_ms(20);  	 	 					
	j = PIND; 								
	if(i == j) 	 	 	 	
	{
		switch (i) 							 
		{
			case  0x7F: 	save=(++save)%2;      break;    //8  led time_set_finish, data write in EEPROM 
			case  0xBF:   CLOCK=(++CLOCK)%3;
			STOP_music=0;  		break;    //7  
			case  0xDF:  sos=(++sos)%3; 	break;	  	  //6->emergency mode	0->forbid    1->allow North_South    2->allow East_West 
		
			
								 
			case  0xEF: 	x=(++x)%3;		break;	  	  //5->select mode    0->general mode    1->night mode    2->emergency mode 
			
			
			
			case  0xF7: 	dx=(++dx)%5;	break;	   	  //4  set_East_West_time       0->red        1->green       2->yellow   3->smile_success
			case  0xFB:   nb=(++nb)%5;      break;   	  //3  set_North_South_time     0->red        1->green       2->yellow   3->smile_success
		
		
			case  0xFE: 								//1
			if(nb==0) a1--;       	//set_North_South_led  minus      
			if(nb==1) b1--;
			if(nb==2) c1--;
			
			if(dx==0) a2--;       	//set_East_West_led    minus     
			if(dx==1) b2--;
			if(dx==2) c2--;
			
			break;
			
			
			
			
			case  0xFD: 			//2 
			if(nb==0) a1++;		  //set_North_South_led  plus
			if(nb==1) b1++;
			if(nb==2) c1++;
		
		
			if(dx==0) a2++;		  //set_East_West_led    plus
			if(dx==1) b2++;
			if(dx==2) c2++;
			break;
		
		
		
		
			default:      break;
		}
			while(PIND!=0xFF)  ; 				//wait button_release
	}
}



/******Interrupt******/
ISR(TIMER1_COMPA_vect)

{
		static unsigned char j=0,c=0; //update_show
	j++;c++;
	k=(++k)%4; 				     	//k update_led


    


	if(j>99)  {shanshuo ^= 1;j=0;}  //point twinkle_cycle_set   1s=0.05*100*2
	
	
	
	
	
	/*************traffic_light time write**************/
	if(c>199)
	{
		c=0;
		if(save==1)      //write traffic_light data in EEPROM
		{
			
			
			
			
		    EEPROM_write(100,a1);
		    EEPROM_write(101,b1);
		    EEPROM_write(102,c1);
		    EEPROM_write(103,a2);
		    EEPROM_write(104,b2);
		    EEPROM_write(105,c2);
		}	
		
	}
	
	
	
		/*******time_set & Numer_update*******/
	if(ms10>99)   
	{
	    ms10=0;
	    
		  
		
		x1=nbred;nbred--;
		x2=dxgreen;dxgreen--;  //100 * 10 =1s
		PORTC = 0x0A;
	   
	
	    if(nbred<0)     
		{
		    x1=nbgreen;nbgreen--;//North_South red->green->yellow 
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
		    x2=dxyellow;dxyellow--;//East_West red->green->yellow
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
	
	
	
	
	
	
	
	
	
	
		/******time_set show*******/
	if(nb==0)     //set North_South red
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

	if(nb==1)   //set North_South green
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

	if(nb==2)		//set North_South yellow
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
	
	if(nb==3)		//success set 
	{
	    dx=-1;x=-1;
		PORTC = 0x00;
		ledbuf[1] = 0x23;
		
		ledbuf[0] = 0x23;
	    ledbuf[2] = 0x23;
	    ledbuf[3] = 0x23;
		display(ledbuf[k],k);
	}
	
	
	
	
	
	
	
	if(dx==0)     //set East_West red
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

	if(dx==1)   //set East_West green
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

	if(dx==2)		//set East_West yellow
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

	if(dx==3)		//success set 
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
	
	
	
	
	
	
	display(ledbuf[k],k);      //led_show function
}






/************AD_Test_voltage, light_control->night_mode************/
unsigned int get_ad(void) 
{ 
   long int i;  
   ADMUX = (1 << REFS0);  //test_voltage, AVCC
   ADCSRA = (1<< ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); //ADC_init,  8 frequency_division
   while(!(ADCSRA & (1 << ADIF))); //wait interrupt_sign 
   i = ADC; //get value
   ADCSRA &= ~(1 << ADIF); //clean  interrupt_sign
   ADCSRA &= ~(1 << ADEN); //ADC  off 
   return i; 
}
 



	
	
	
	
	
	
/************EEPROM write************/

void EEPROM_write(unsigned int Address,unsigned char Data)
{
	while(EECR&(1<<EEWE));			//wait ending
	EEAR = Address;  				//address
	EEDR = Data;  					//data
	EECR |= (1<<EEMWE);  			//EEMWE
	EECR |= (1<<EEWE);				//EEWE write
}





/************EEPROM read************/
int EEPROM_read(unsigned int Address)
{
	while(EECR&(1<<EEWE));			//wait ending
	EEAR = Address;  				//address
	EECR |= (1<<EERE); 				//EEMWE
	return EEDR;  					//return read_data
}





//main 
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
	
	
	

   
	
	
		
	





	
	
	
    	/********read traffic_light_time*********/

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
        
		key_read(); 	 	//scan keyboard
		delay_ms(50); 		
		
		
        i = get_ad()/204.8;


		
		if(x==0)//general_mode 
		{
			T=-1;sos=-1;			
	
	      if(i>2)                //AD_Test_voltage, light_control
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
		
		
		if(x==1)//night_mode
		{

            T=-1;sos=-1;			
            ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;

			if(shanshuo==1)
			PORTC = 0x24;
			else
			PORTC = 0x00;
			
		} 
		
		
		
		if(x==2)//emergency_mode 
		{
			nb=-1;dx=-1;T=-1;;		
			ledbuf[3] = 0x00;
	        ledbuf[2] = 0x00;
        	ledbuf[1] = 0x00;
	        ledbuf[0] = 0x00;
			PORTC = 0x09;
			if(sos==0) PORTC = 0x09;
			if(sos==1) PORTC = 0x0A;
			if(sos==2) PORTC = 0x11;
			
			
		} 
			
	 
	    

    

	      
	
	
	    
			
	
		delay_ms(10); 
	}
	
	
	
} 
	
	
	
	
	
	

	
	
	
	
	
	
