/* =================================================================================================
   bt_led_control - controlling an LED connected to an arduino using text commands received from
                    a serial line, preferably one with a bluetooth module attached to it.
                 
   The program assumes that you have to things: A serial connection, e.g. with a bluetooth module
   and an LED connected to a PWM enabled pin of the arduino. Now you can send commands over the
   serial line. The response on the line will be a status code (inspired by http) followed by the
   command itself. A typical communication process might look like this:
   
   ON
   200 ON
   OFF
   200 OFF
   LEVEL 127
   200 LEVEL 127
   FOOBAR
   405 FOOBAR
   
   The commands may have an arbitrary amount of arguments.
   
   Commands that return a value act a little different. Example:
   
   ON
   200 ON
   STATUS
   200 STATUS 255
   LEVEL 128
   200 LEVEL 128
   STATUS
   200 STATUS 128
   
   As you can see, the return value is simply added to the echoed command.
   
   This code was written by Gregor Ottmann. It is public domain. Neither strings nor warranties
   of any kind are attached.
================================================================================================= */


// the LED is connected to this pin. use a pwm capable pin for this as the
// LEVEL command won't work otherwise.
#define LED_PIN 3

// this is the maximum size for any command including its parameters
#define CMD_BUFFER_SIZE 16

// if one has an arduino mega, it would be possible to use a different
// serial port for the bluetooth module. or just use this one and debug using
// the usb-serial monitor in the arduino IDE
//
// On an Arduino Mega, "Serial1" on Pins 18/19 is a good thing as it needn't be
// disconnected for programming over USB. On an Arduino with only one serial line
// or when using the USB connection you'll have to use "Serial".
#define COMPORT Serial1

// the speed at which the serial communication should take place
#define COMSPEED 9600

// base used when converting parameter strings to numbers. normally 10 or 16.
#define NUMBER_BASE 10

// the commands coming from the serial port will be stored here
char  commandBuffer[CMD_BUFFER_SIZE+1];

// this is the position in the buffer at which the next character from
// the serial port will be written.
int   commandCursor = 0;

// the current level of the LED which will be returned by the STATUS command
int currentLevel = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  COMPORT.begin(COMSPEED);
}


void loop() {
  while(COMPORT.available())
  {
    char current = (char)COMPORT.read();
    
    // if the command is terminated (10 == newline) or if a space is encountered,
    // a 0 char is written to the buffer. this makes it extremely easy to parse
    // the components later on - see parseNumber()
    if(current == 10 || current == 13 || current == ' ')
      commandBuffer[commandCursor++] = 0;
    else
      commandBuffer[commandCursor++] = current;

    // if the maximum command length is reached or if the command is terminated with a new line
    // character, the command buffer is terminated with a 0 chat and the command processing
    // routine is called.    
    if(commandCursor >= CMD_BUFFER_SIZE || current == 10 || current == 13) {
      commandBuffer[CMD_BUFFER_SIZE] = 0;
      commandCursor = 0;
      processCommand();
    }
  }
}

/**
 * Process the command that was read from the serial port.
 *
 * Valid commands are "ON", "OFF", "LEVEL <level>" and "STATUS".
 */
void processCommand() {
  // uppercase the command
  char *p = commandBuffer;
  while(*p != 0) {
    *p = toupper(*p);
    p++;  
  }
  
  if(strcmp(commandBuffer, "ON") == 0) {
    COMPORT.println("200 ON");
    digitalWrite(LED_PIN, HIGH);
    currentLevel = 255;
  }
  else if(strcmp(commandBuffer, "OFF") == 0) {
    COMPORT.println("200 OFF");
    digitalWrite(LED_PIN, LOW);
    currentLevel = 0;
  }
  else if(strcmp(commandBuffer, "STATUS") == 0) {
    COMPORT.print("200 STATUS ");
    COMPORT.println(currentLevel);
  }
  else if(strcmp(commandBuffer, "LEVEL") == 0) {
    COMPORT.print("200 LEVEL ");
    
    // this command has a numeric parameter directly behind the
    // command itself.
    char *parsePointer = commandBuffer + strlen(commandBuffer) + 1;
    currentLevel = constrain(parseNumber(&parsePointer), 0, 255);
    
    
    // no need to treat out-of-range-numbers as errors ... just
    // cap the range.
    if(currentLevel < 0)
      currentLevel = 0;
    else if(currentLevel > 255)
      currentLevel = 255;

    COMPORT.println(currentLevel);
    analogWrite(LED_PIN, currentLevel);
  }
  else if(strcmp(commandBuffer, "FADE") == 0) {
    COMPORT.print("200 FADE ");
    
    // this command has a numeric parameter directly behind the
    // command itself.
    char *parsePointer = commandBuffer + strlen(commandBuffer) + 1;
    int targetLevel = constrain(parseNumber(&parsePointer), 0, 255);
    int fadeDelay = constrain(parseNumber(&parsePointer), 0, 100);
    
    
    COMPORT.print(targetLevel);
    COMPORT.print(" ");
    COMPORT.println(fadeDelay);

    while(currentLevel != targetLevel)
    {
      currentLevel = currentLevel < targetLevel ? currentLevel + 1 : currentLevel - 1;
      analogWrite(LED_PIN, currentLevel);
      delay(fadeDelay);
    }
  }
  else {
    COMPORT.print("405 ");
    COMPORT.println(commandBuffer);
  }
  
  // clear the buffer so we don't risk having some old crap there when
  // parsing ...
  for(int i = 0; i < sizeof(commandBuffer); i++)
    commandBuffer[i] = 0;
}

/**
 * Parses a number that is represented by a 0-terminated string.
 *
 * The argument pointer will be dereferenced and the string at that location
 * will be parsed using atoi(). After that the dereferenced pointer is increased so
 * it points to the first byte AFTER the terminating zero of the string that was just
 * parsed.
 */
int parseNumber(char** parsePointer) {
  int result = strtol(*parsePointer, NULL, NUMBER_BASE);
  *parsePointer+=(strlen(*parsePointer) + 1);
  
  return result;
}
