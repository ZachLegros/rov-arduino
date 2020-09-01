#include <SPI.h>
#define BUFFER_SIZE 16
#define MAX_CONTROL_VALS 10

char sensor_vals[5] = {1, 2, 3, 4, 5};
char buf[BUFFER_SIZE];
volatile byte pos;
volatile boolean process_it;

void print_buffer()
{
  for (int i=0; i<BUFFER_SIZE; i++)
  {
    Serial.print(buf[i], DEC);
  }
  Serial.println();
}

void setup()
{
  Serial.begin (38400);   // debugging

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;

  // turn on interrupts
  SPI.attachInterrupt();

}

// SPI interrupt routine
ISR(SPI_STC_vect)
{
  byte c = SPDR;  // grab byte from SPI Data Register
  
  // add to buffer if room
  if (pos < BUFFER_SIZE)
  {
    buf[pos] = c;
    
    if (c == '\n')
      process_it = true;
      
  }
  
  if (pos >= MAX_CONTROL_VALS && pos - MAX_CONTROL_VALS < 5) 
  {
    SPDR = sensor_vals[pos - MAX_CONTROL_VALS];
  }
  else
  {
    SPDR = 0;
  }

  pos++;
}  

void loop()
{
  if (process_it)
  {
    buf[pos] = 0;  
    print_buffer();
    pos = 0;
    process_it = false;
  }
}
