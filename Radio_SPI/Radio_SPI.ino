#include <SPI.h>
#include "definition.h"

#define csn_pin 8
#define payload_size 32

// RF modules support 10 Mhz SPI bus speed
const uint32_t RF24_SPI_SPEED = 10000000;
const byte rxAddr[6] = "00001";

int result;
int status;
int pipe0_reading_address;

void startData( void );
void transferData( void );


void setup() {
  SPI.begin();
  Serial.begin(9600);
  powerUp();
  openReadingPipe(0, rxAddr);
}

void loop() {

}

void startData( void ) {
  SPI.beginTransaction(SPISettings(RF24_SPI_SPEED, MSBFIRST, SPI_MODE0));
}
void endData( void ){
  SPI.endTransaction();
}
void csn(int mode)
{
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  digitalWrite(csn_pin,mode);
}
uint8_t readData( uint8_t reg ) { //Done
  SPI.transfer( R_REGISTER | ( REGISTER_MASK & reg ));
  result = SPI.transfer(0xff);
  endData();
  return result;
}
uint8_t writeData( uint8_t reg,uint8_t value ) { //Done
  startData();
  status = SPI.transfer( R_REGISTER | ( REGISTER_MASK & reg ));
  SPI.transfer(value);
  endData();
  return status;
}
uint8_t writeData_extra(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;
  csn(LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    SPI.transfer(*buf++);

  csn(HIGH);

  return status;
}



void powerUp(void)
{
   uint8_t cfg = readData(NRF_CONFIG);
   if (!(cfg & _BV(PWR_UP))){
      writeData(NRF_CONFIG, cfg | _BV(PWR_UP));   
      delay(5);
   }
}

void openReadingPipe(uint8_t child, uint64_t address)
{
  if (child == 0)
    pipe0_reading_address = address;
  if (child <= 6)
  {
    if ( child < 2 )
      printData("Status 1",writeData_extra(pgm_read_byte(&child_pipe[child]), reinterpret_cast<const uint8_t*>(&address), 5));
    else
      printData("Status 2",writeData_extra(pgm_read_byte(&child_pipe[child]), reinterpret_cast<const uint8_t*>(&address), 1));
    printData("Status 3",writeData(pgm_read_byte(&child_payload_size[child]),payload_size));
    printData("Status 4",writeData(EN_RXADDR,readData(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[child]))));
  }
}
void printData(char string[], int data){
  Serial.print(string);
  Serial.print(": ");
  Serial.println(data);
}

