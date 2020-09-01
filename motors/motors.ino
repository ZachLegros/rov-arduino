#define enA 9
#define in1 4
#define in2 5
#define enB 10
#define in3 6
#define in4 7
#define BUFFER_SIZE 4

int incomingByte = 0; // for incoming serial data
int buffer[BUFFER_SIZE] = {0, 0, 0, 0};
int index = 0;
int motorSpeedA = 0;
int motorSpeedB = 0;
int directionAB = 0;

void printBuffer(int buffer[])
{
  for (int i=0; i<BUFFER_SIZE; i++)
  {
    Serial.print(buffer[i]);
    Serial.print(" ");
  }
  Serial.println();
}


void getBuffer()
{
  bool bufferFull = false;
  
  while (!bufferFull)
  {
    // read the incoming byte:
    incomingByte = int(Serial.read());
    if (incomingByte == 9)
    {
      index = 0;
      bufferFull = true;
    }
    else if (incomingByte > -1 && incomingByte < 256)
    {
      buffer[index] = incomingByte;
      index++;
    }
  }
}


void flushBuffer(int buffer[])
{
  for (int i=0; i<BUFFER_SIZE; i++)
  {
    buffer[i] = 0;  
  }
}


void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
}

void loop() {
  getBuffer();

  if (directionAB != buffer[1])
  {
    directionAB = buffer[1];
    if (directionAB == 1)
    {
      // backward
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      delay(20);
    }
    else
    {
      // forward
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      delay(20);
    }
  }
  
  motorSpeedA = buffer[0];
  analogWrite(enA, motorSpeedA); // Send PWM signal to motor A
  //analogWrite(enB, motorSpeedB); // Send PWM signal to motor B
  
//  flushBuffer(buffer);
}
