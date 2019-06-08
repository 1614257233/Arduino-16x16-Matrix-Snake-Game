#include <Arduino.h>

//Input and Output Pins
#define LEDARRAY_D 2
#define LEDARRAY_C 3
#define LEDARRAY_B 4
#define LEDARRAY_A 5
#define LEDARRAY_G 6
#define LEDARRAY_DI 7
#define LEDARRAY_CLK 8
#define LEDARRAY_LAT 9

#define KEY_Right 11      //Right
#define KEY_Down 13     //Down
#define KEY_Left 10     //Left
#define KEY_Up 12     //Up

#define Right 0
#define Left 2
#define Up 3
#define Down 1

//#include <stdlib.h>

unsigned char Display_Buffer[2];
unsigned char Shift_Count = 0;
unsigned char table[31]={0x00,0x10,0x20};        //The coordinate of the body of the snake
unsigned char count = 3;                       //Length of the snake
unsigned char Direction;
unsigned char T;
int tt;
unsigned char Flag_Shift,Flag_Over,flag3,Flag_Dot,Flag_Draw;
unsigned char u;

bool Shift_Bit = 0;
bool Flag_Word = true;
unsigned char Display_Swap_Buffer[1][32]={0};          //Buffer
unsigned char temp_word = 0x80;
unsigned char Display_Word_Count = 0;
#define Num_Of_Word 3
const unsigned char Word[Num_Of_Word][32] =         // http://66robo.blogspot.com/2012/08/led-excel.html
{
    0xFF,0xFF,0xFF,0xC1,0xBD,0xBD,0xBF,0xDF,0xE7,0xFB,0xFD,0xBD,0xBD,0x83,0xFF,0xFF,/*"S"*/
    0xFF,0xFF,0xFF,0x9E,0x9E,0xAE,0xAE,0xB6,0xB6,0xB6,0xBA,0xBA,0xBC,0xBC,0xFF,0xFF,/*"N"*/
    0xFF,0xFF,0xFF,0xEF,0xEF,0xE7,0xD7,0xD7,0xDB,0xC3,0xBB,0xBD,0xBD,0x18,0xFF,0xFF,/*"A"*/
    0xFF,0xFF,0xFF,0xBF,0xBE,0xBD,0xB3,0xAF,0x8F,0xB7,0xBB,0xBD,0xBE,0xBF,0xFF,0xFF,/*"K"*/
    0xFF,0xFF,0xFF,0x80,0xBF,0xBF,0xBF,0xBF,0x81,0xBF,0xBF,0xBF,0xBF,0x80,0xFF,0xFF,/*"E"*/
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,/*" "*/
};

void setup()
{
    pinMode(LEDARRAY_D, OUTPUT); 
    pinMode(LEDARRAY_C, OUTPUT);
    pinMode(LEDARRAY_B, OUTPUT);
    pinMode(LEDARRAY_A, OUTPUT);
    pinMode(LEDARRAY_G, OUTPUT);
    pinMode(LEDARRAY_DI, OUTPUT);
    pinMode(LEDARRAY_CLK, OUTPUT);
    pinMode(LEDARRAY_LAT, OUTPUT);

    pinMode(KEY_Right, INPUT);       //All the buttons
    pinMode(KEY_Left, INPUT);
    pinMode(KEY_Up, INPUT);
    pinMode(KEY_Down, INPUT);

    Init();
    Clear_Display();
}

void loop()
{
  if (Flag_Word){                         // Displaying the words
      unsigned int i;
      for(i = 0 ; i < 30; i ++)     
      {
        Display_Word(Display_Swap_Buffer);
      }
      Display_Word_Count = Shift_Count/16;        //Which word
      Calc_Shift();
      Shift_Count++;
      if(Shift_Count == (Num_Of_Word+1)*16 )        //How many times they have shifted
      {
        Shift_Count = 0;        
      } 
      Scan_Key();
  
  }
  else{                                   //Playing the game
    if (Flag_Dot == 1)                  //Press DOWN to start
        Random_Dot();                       //A random dot is shown on the matrix
      if (Flag_Shift == 1)    
        Shift();                             //The snake starts moving   
      Scan_Key();
      Display();  
  }
    

}


//**********************************************************
//Time interrupt routine, to shift the snake
//**********************************************************
ISR(TIMER1_COMPA_vect) //Interrupt timer 1 and make the snake available to shift
{
    T++;
    if (T>=5)
    {
      T=0;
      Flag_Shift=1;
    }   

}

//************************************************************
//Clear the Buffer (Displaying the words)
//************************************************************
void Clear_Display()
{
    unsigned char i;
  
    for(i = 0 ; i < 32 ;i++)
    {
      Display_Swap_Buffer[0][i] = 0xff;       //0= Showing  1= Not Showing   
    }
}

//************************************************************
//Calculate the shifting data of the words
//************************************************************
void Calc_Shift()
{
  unsigned char i;

    for(i = 0;i < 16;i++)
    {
      if((Display_Swap_Buffer[0][16+i]&0x80) == 0)                  //Shifting the first 8 
      {
          Display_Swap_Buffer[0][i] = (Display_Swap_Buffer[0][i] << 1)&0xfe;      //make the last digit become 0
      }
      else
      {
          Display_Swap_Buffer[0][i] = (Display_Swap_Buffer[0][i] << 1)|0x01;      //make the last digit become 1
      }

      if(Shift_Count%16 < 8 && Display_Word_Count < Num_Of_Word)    
      {
          Shift_Bit = Word[Display_Word_Count][i]&temp_word;
      }
      else if(Shift_Count%16 < 16 && Display_Word_Count < Num_Of_Word)
      {
          Shift_Bit = Word[Display_Word_Count][16+i]&temp_word;
      }
      else
      {
          Shift_Bit = 1;            //Get the word out of the Buffer
      }

      if( Shift_Bit == 0)                           //Shifting the last 8 
      {
          Display_Swap_Buffer[0][16+i] = (Display_Swap_Buffer[0][16+i] << 1)&0xfe;    //The lowest digit becomes 0
      }
      else
      {
        Shift_Bit = 1;
          Display_Swap_Buffer[0][16+i] = (Display_Swap_Buffer[0][16+i] << 1)|0x01;    //The lowest digit becomes 1    
      }
    
    }
    temp_word = (temp_word>>1)&0x7f;
    if(temp_word == 0x00)
    {
      temp_word = 0x80;
    }
}

//************************************************************
//num is the number of words,  dat[][32]is the buffer
//*************************************************************
void Display_Word(unsigned char dat[][32])         
{
    unsigned char i;

    for( i = 0 ; i < 16 ; i++ )
    {
      digitalWrite(LEDARRAY_G, HIGH);   //When updating the data,please close the display. After updating the data, open the 138 display line. Avoiding ghosting.
    
      Display_Buffer[0] = dat[0][i];    
      Display_Buffer[1] = dat[0][i+16];

      Send(Display_Buffer[1]);
      Send(Display_Buffer[0]);

      digitalWrite(LEDARRAY_LAT, HIGH);         //Lock the Data
      delayMicroseconds(1);
  
      digitalWrite(LEDARRAY_LAT, LOW);
      delayMicroseconds(1);

      Scan_Line(i);           //Scan Line i

      digitalWrite(LEDARRAY_G, LOW);
    
      delayMicroseconds(300);    //Delay, and then light up the LED      
    } 
}

//**********************************************************
//Move the snake
//**********************************************************
void Shift()
{
  int k;

  if ( Flag_Draw == 1 )
  {
    for(k=0;k<count-1;k++)        //Drawing the snake
    {
      table[k]=table[k+1];
    }
  }
  
    switch (Direction)
    {
      case 0:             //RIGHT
          if (table[count-1]/16<15)       //The snake head didn't reach the right boundary
            table[count-1]=table[count-1]+16; // Go Right
          else
            Flag_Over=1;              //The snake head reached the right boundary, GAME OVER
          break;
      case 1:             //DOWN
          if (table[count-1]%16<15)        //The snake head didn't reach the down boundary
            table[count-1]=table[count-1]+1;
          else
            Flag_Over=1;                    // Game over
          break;
      case 2:             //LEFT
          if (table[count-1]/16>0)
            table[count-1]=table[count-1]-16;  // The snake head didn't reach the left boundary
          else
            Flag_Over=1;                     // Game over
          break;
      case 3:             //UP
          if (table[count-1]%16>0)
            table[count-1]=table[count-1]-1;    // The snake didn't reach the up boundary
          else
            Flag_Over=1;                    // Game over
          break;
      default:
          break;
  }
    Flag_Shift=0;
    if (Flag_Over == 0)
    {
      for (k=0;k<count-1;k++)             //Check if the head of the snake touches itself
      {
          if (table[count-1]==table[k])
          {
            Flag_Over=1;
            break;
          }
      }
    }
    if (Flag_Over == 1)
    { 
      cli();
      Flag_Dot = 2;
      ReInit();         //Restart the game
    }
    else if (table[count-1]==table[count])    //The head of the snake touches the food
    {
      if (count<30)             
      {
        count++;              //The length of the snake + 1
        Flag_Draw=0;
      }
      Flag_Dot=1;
      table[count]=0x00;            // New food will not be shown on the matrix before its coordinate is randomized
    }
    else
    {
      Flag_Draw=1;
    }


}

//**********************************************************
//The random function for the food 
//**********************************************************
void Random_Dot()
{
    int k;
    Flag_Dot=0;
    randomSeed(analogRead(0));
    do
    {
      flag3=1;
      u=random(256);        // A random number between 0-255
      u=u/16*10+u%16;       //Take out the first four digits as the row number, the last four digits as the column number
      for (k=0;k<count;k++)   // See if it is repeated
      {
          if (u==table[k])    //If it's on the snake, randomize again
          {
            flag3=0;
            break;
          }
      }
    }
    while(!flag3);				//When flag3 == 1, it means that the random dot is picked
                               
    table[count]=u;         //Put the random dot into table[count]
}

void Init_time()
{
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
 
    // set compare match register to desired timer count:
    OCR1A = 781;          //20ms
                  //15624  1S
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    //sei();          // enable global interrupts
}

//**********************************************************
//Reinitialnizing the snake game
//**********************************************************
void ReInit()
{ 
    table[0]=0x00;
    table[1]=0x10;
    table[2]=0x20;
    delay(500);
    Flag_Over=0;
    Flag_Dot=2;
    Flag_Draw=1;
    count=3;
    table[count]=0;
    Flag_Shift=0;

    Direction=0;
  
    Init_time();
}
//**********************************************************
//Initializing the snake game
//**********************************************************
void Init()
{
    Flag_Shift=0;
    Flag_Draw=1;
    Flag_Dot=2;
    tt=0;
    T=0;
    Flag_Over=0;

    count=3;          //Initial length of the snake is 3  
    Direction=0;            //Initial direction

    Init_time();
}

//**********************************************************
//Scanning all the keys
//**********************************************************
void Scan_Key()
{
    if(digitalRead(KEY_Right) == 1)
    {
      if (Flag_Word){
        Flag_Word = false;
      }
    
      if (Flag_Dot==2)
        {
            Flag_Dot=1;     //Start the Game
            sei();        //Start interrupting and start moving
        }
        if(Direction != Left){
            Direction = Right;} 
    
  }

    if(digitalRead(KEY_Down) == 1)
    {
      if(Direction != Up){
          Direction = Down; }
    }

    if(digitalRead(KEY_Left) == 1)
    {
      if(Direction != Right){
          Direction = Left;}
  
    }

    if(digitalRead(KEY_Up) == 1)
    {
      if(Direction != Down){
          Direction = Up; }
    }
}



//************************************************************
//Displaying the snake game
//*************************************************************
void Display()          
{
    unsigned char i,j;
    unsigned int temp = 0x7fff;
    unsigned char x,y;

    for( j = 0 ; j <= count ; j++ )
    {
      digitalWrite(LEDARRAY_G, HIGH);     //When updating the data,please close the display. After updating the data, open the 138 display line. Avoiding ghosting.

      y=table[j]/16;        //The higher 4 digits is the column number       0 = on, 1 = off
      x=table[j]%16;        //The lower 4 digits is the row number         1 = on, 0 = off


      temp = 0x7fff;    
      for(i = 0 ; i < y ; i++)
      {
          temp = (temp>>1)|0x8000;
      } 

      Display_Buffer[1] = temp&0x00ff;
      Display_Buffer[0] = (temp>>8)&0x00ff;
        
      Send(Display_Buffer[1]);
      Send(Display_Buffer[0]);

      digitalWrite(LEDARRAY_LAT, HIGH);         //Lock the Data
      delayMicroseconds(1);
  
    
      digitalWrite(LEDARRAY_LAT, LOW);
      delayMicroseconds(1);

      Scan_Line(x);           //Choose Line i

      digitalWrite(LEDARRAY_G, LOW);
    
      delayMicroseconds(300);    //Delay, light up the LED      
    }
}

//****************************************************
//Scanning lines
//****************************************************
void Scan_Line( unsigned char m)
{ 
    switch(m)
    {
      case 0:     
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, LOW);          
          break;
      case 1:         
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, HIGH);     
          break;
      case 2:         
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, LOW);     
          break;
      case 3:         
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, HIGH);    
          break;
      case 4:
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, LOW);     
          break;
      case 5:
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, HIGH);    
          break;
      case 6:
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, LOW);    
          break;
      case 7:
          digitalWrite(LEDARRAY_D, LOW);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, HIGH);     
          break;
      case 8:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, LOW);     
          break;
      case 9:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, HIGH);    
          break;  
      case 10:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, LOW);    
        break;
      case 11:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, LOW);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, HIGH);     
      break;
      case 12:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, LOW);    
          break;
      case 13:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, LOW);digitalWrite(LEDARRAY_A, HIGH);     
          break;
      case 14:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, LOW);     
          break;
      case 15:
          digitalWrite(LEDARRAY_D, HIGH);digitalWrite(LEDARRAY_C, HIGH);digitalWrite(LEDARRAY_B, HIGH);digitalWrite(LEDARRAY_A, HIGH);    
          break;
      default : break;  
  }
}

//****************************************************
//Sending Data
//****************************************************
void Send( unsigned char dat)
{
    unsigned char i;
    digitalWrite(LEDARRAY_CLK, LOW);
    delayMicroseconds(1);
    digitalWrite(LEDARRAY_LAT, LOW);
    delayMicroseconds(1);

    for( i = 0 ; i < 8 ; i++ )
    {
      if( dat&0x01 )
      {
          digitalWrite(LEDARRAY_DI, HIGH);  
      }
      else
      {
          digitalWrite(LEDARRAY_DI, LOW);
      }


      digitalWrite(LEDARRAY_CLK, HIGH);       //Sending data
        delayMicroseconds(1);
      digitalWrite(LEDARRAY_CLK, LOW);
        delayMicroseconds(1);    
      dat >>= 1;
      
  }     
}
