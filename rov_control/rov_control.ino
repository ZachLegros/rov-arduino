#include <SPI.h>
// motor pins
#define enA 2
#define in1 22
#define in2 23
#define enB 3
#define in3 24
#define in4 25
#define enC 4
#define in5 26
#define in6 27
#define enD 5
#define in7 28
#define in8 29
// constants
#define BUFFER_SIZE 16
#define MAX_CONTROL_VALS 10
#define SENSOR_VALS_SIZE 5

unsigned char sensor_vals[SENSOR_VALS_SIZE] = {1, 2, 3, 4, 5};
unsigned char buf[BUFFER_SIZE];
volatile byte pos;
volatile boolean process_it;
int motorSpeedA = 0; // left
int motorSpeedB = 0; // right
int motorSpeedC = 0;
bool directionA = 0;
bool directionB = 0;
bool directionCD = 0;

void print_buffer()
{
  for (int i=0; i<BUFFER_SIZE; i++)
  {
    Serial.print(buf[i], DEC);
  }
  Serial.println();
}

void print_motors()
{
  Serial.print("A: ");
  Serial.print(motorSpeedA);
  Serial.print(" B: ");
  Serial.print(motorSpeedB);
  Serial.print(" CD: ");
  Serial.print(motorSpeedC);
  Serial.println();
  Serial.print("dir A: ");
  Serial.print(directionA);
  Serial.print(" dir B: ");
  Serial.print(directionB);
  Serial.print(" dir CD: ");
  Serial.print(directionCD);
  Serial.println();
}

void read_sensors()
{
  // temporary simulating changing sensor values  
  int q[2] = {0, };
  for (int i = 0; i < SENSOR_VALS_SIZE; i++)
  {
    if (i == 0)
    {
      q[0] = sensor_vals[0];
      sensor_vals[0] = sensor_vals[SENSOR_VALS_SIZE-1];
    }
    else
    {
      q[1] = sensor_vals[i];
      sensor_vals[i] = q[0];
      q[0] = q[1];
    }
  }
}

int invert_pwm(int pwm)
{
  switch (pwm)
  {
  case 0:
    return 255;
    break;
  case 63:
    return 191;
    break;
  case 127:
    return 127;
    break;
  case 191:
    return 63;
    break;
  case 255:
    return 0;
    break;
  default:
    return 0;
  }
}

void set_direction(int dir, int inA, int inB)
{
  if (dir == 1)
  {
    // backward
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
  }
  else
  {
    // forward
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
  }
}

void update_motors()
{
  // forward/backward motors
  directionA = directionB = buf[1];

  // turn only
  if (buf[0] == 0 && buf[4] != 0)
  {
    // left
    if (buf[5] == 1)
    {
      directionA = !directionA;
      set_direction(directionA, in1, in2);
    }
    // right
    else
    {
      directionB = !directionB;
      set_direction(directionB, in3, in4);
    }
    motorSpeedA = motorSpeedB = buf[4];
  }
  // move forward and turn simultaneously
  else if (buf[0] != 0 && buf[4] != 0)
  {
    // left
    if (buf[5] == 1)
    {
      motorSpeedA = invert_pwm(buf[4]);
      motorSpeedB = buf[0];
    }
    // right
    else
    {
      motorSpeedB = invert_pwm(buf[4]);
      motorSpeedA = buf[0];      
    }
    set_direction(directionA, in1, in2); // motor A
    set_direction(directionB, in3, in4); // motor B
  }
  // move forward
  else
  {
    set_direction(directionA, in1, in2); // motor A
    set_direction(directionB, in3, in4); // motor B
    motorSpeedA = motorSpeedB = buf[0];
  }

  // up/down motors
  if (directionCD != buf[3])
  {
    // up/down motors
    directionCD = buf[3];
    set_direction(directionCD, in5, in6); // motor C
    set_direction(directionCD, in7, in8); // motor D
  }
  motorSpeedC = buf[2];
  
  analogWrite(enA, motorSpeedA); // Send PWM signal to motor A
  analogWrite(enB, motorSpeedB); // Send PWM signal to motor B
  analogWrite(enC, motorSpeedC); // Send PWM signal to motor C
  analogWrite(enD, motorSpeedC); // Send PWM signal to motor D
}

void setup()
{
  // motor pins
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(enC, OUTPUT);
  pinMode(enD, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(in5, OUTPUT);
  pinMode(in6, OUTPUT);
  pinMode(in7, OUTPUT);
  pinMode(in8, OUTPUT);

  // SPI
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
    read_sensors();
    update_motors();
    print_motors();
    pos = 0;
    process_it = false;
  }
}
