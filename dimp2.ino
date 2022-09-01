#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU 1000000UL // 1mhz
#define MUX_OFFSET 0x40 // MUX value for REFS0 to be enabled for AREV <---> VCC tie
#define ADC_SAVE 64 // average ADC values for sensors

#define CC_D1 PB0
#define CC_D2 PB1
#define CC_D3 PB2
#define CC_D4 PB6
#define ADC0 PC0

#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))

// globals
static uint16_t adc_volt;
static uint16_t adc_read;
static uint16_t adc_avg_total;
static uint8_t  adc_count; // counts sensor averaging
static uint16_t avg_adc[ADC_SAVE]; // store adc values for each sensor for average calc
static uint8_t  volt_digits[4]; // digits are logically numbered 0 to 3, starting from left
static uint8_t  decimal_digit = 2; // indicates which digit from 0 to 3 has the decimal point

//
//  pretty close to 1ms delay at 1MHz...
//
//static void delay_ms(unsigned char ms)
//{
//  volatile int i;
//
//  while (ms--)
//    //for (i = 0; i < 54; i++)
//    for (i = 0; i < 13; i++)
//      ;
//}

static void delay_ms(int delay)
{
  for (int i=0;i<delay;i++)
    _delay_ms(1);
}

ISR(ADC_vect)
{
  // collect analog values for averaging
  avg_adc[adc_count] = ADCW;
  adc_count++;
}

static void adc2avg(void)
{
  // average the sensor data
  static uint8_t i;

  // reset global vars to zero
  adc_count = 0;
  adc_volt = 0;
  adc_read = 0;
  adc_avg_total = 0;

  // slow down and average ADC readings to reduce noise
  // reduce ADC_SAVE to speed up response
  for (i = 0; i < ADC_SAVE; i++)
  {
    adc_avg_total += avg_adc[adc_count];
  }

  adc_read += adc_avg_total / ADC_SAVE;
  //adc_volt = adc_read;
}

static void adc2volt(void)
{
  // convert adc values into voltage
  //
  // based on our voltage divider with R1 = 24.9k and R2 = 470 and maximum Vin into the voltage divider = 250V, the max Vout sent to Vin into the ADC0 is 4.6314544737V
  // VREF is supposed to be 5.05V with a fresh 9V battery
  // max ADC output = (Vin * 1024) / VREF
  // so max ADC output we can expect is (4.6314544737 * 1024) / 5.05 = 939
  // thus we can expect the ADC value to be in the range 0 to 939
  // each ADC unit will then be 250V / 939 = 0.26624068157V

  adc_volt = adc_read * 26.624; // we're multiplying by the ADC unit voltage and then by 100 so we can use integers and still get hundredths

  // super-cheesy multipliers to adjust the displayed value to match measurements of real-world batteries of known voltages
  if (adc_read <= 10) // batteries 0 to 2.66V
    adc_volt = adc_volt * 1.06;
  else if (adc_read > 10 && adc_read <= 28) // batteries 2.66 to 7.45V
    adc_volt = adc_volt * 1.045;
  else if (adc_read > 28 && adc_read <= 57) // batteries 7.45 to 15V
    adc_volt = adc_volt * 1.03;
  else if (adc_read > 57 && adc_read <= 94) // batteries 15 to 25V
    adc_volt = adc_volt * 1.01;
  else if (adc_read > 94) // batteries 25V and up
    adc_volt = adc_volt * 0.95;
}

static void digit_breakup(void)
{
  //TODO show hundredths place if voltage < 100
  // break voltage up into separate digits
  if ((adc_volt / 10000) % 10 == 0) {
    // volage < 100, decimal point should be after digit 1 (2nd from left)
    volt_digits[3] = adc_volt % 10;
    volt_digits[2] = (adc_volt / 10) % 10;
    volt_digits[1] = (adc_volt / 100) % 10;
    volt_digits[0] = (adc_volt / 1000) % 10;
    decimal_digit = 1;
  }
  else {
    // voltage > 100, decimal point should be after digit 2 (3rd from left)
    volt_digits[3] = (adc_volt / 10) % 10;
    volt_digits[2] = (adc_volt / 100) % 10;
    volt_digits[1] = (adc_volt / 1000) % 10;
    volt_digits[0] = (adc_volt / 10000) % 10;
    decimal_digit = 2;
  }
}

static uint8_t digit_to_hex(uint8_t display_digit)
{
  uint8_t hex;
  switch (display_digit)
  {
  case (0): { hex = 0xFC; break; }
  case (1): { hex = 0x50; break; }
  case (2): { hex = 0x6E; break; }
  case (3): { hex = 0x76; break; }
  case (4): { hex = 0xD2; break; }
  case (5): { hex = 0xB6; break; }
  case (6): { hex = 0xBE; break; }
  case (7): { hex = 0x70; break; }
  case (8): { hex = 0xFE; break; }
  case (9): { hex = 0xF6; break; }
  }
  return(hex);
}

static void write_sev_seg(uint8_t hexcode, uint8_t digit)
{
  // enable specified digit by pulling its common cathode low
  switch (digit) {
  case (0): { CLR(PORTB, CC_D1); break; }
  case (1): { CLR(PORTB, CC_D2); break; }
  case (2): { CLR(PORTB, CC_D3); break; }
  case (3): { CLR(PORTB, CC_D4); break; }
  }

  PORTD = hexcode;

  // controls display speed for specific digits
  if (PORTD == 0x50) {
    // the digit 1 is too bright if the delay is the same as the other digits
    delay_ms(1);
  }
  else if (PORTD == 0x70) {
    // digit 7
    delay_ms(2);
  }
  else {
    delay_ms(3);
  }

  // output for display, start high, pull low to use
  SET(PORTB, CC_D1);
  SET(PORTB, CC_D2);
  SET(PORTB, CC_D3);
  SET(PORTB, CC_D4);
}

static void display_digits(int8_t dpPos)
{
  int8_t disp_pos;

  for (disp_pos = 0; disp_pos <= 3; disp_pos++)
  {
    uint8_t hexcode = digit_to_hex(volt_digits[disp_pos]);

    if (disp_pos == dpPos)
      hexcode |= 0x01; // adds dp dot

    write_sev_seg(hexcode, disp_pos);
  }
}

static void port_init(void) {
  // Port D is for the 7-segment anodes
  DDRD = 0xFF; // set every pin of Port D to output mode
  PORTD = 0x00; // disable pull-up on every pin of Port D

  // some of the pins of Port B are for the common cathodes for each digit
  // they start high, pull low to use
  SET(DDRB, CC_D1); // set this pin to output mode
  SET(DDRB, CC_D2);
  SET(DDRB, CC_D3);
  SET(DDRB, CC_D4);
  SET(PORTB, CC_D1); // disable pull-up on this pin
  SET(PORTB, CC_D2);
  SET(PORTB, CC_D3);
  SET(PORTB, CC_D4);
}

static void adc_init(void)
{
  SET(ADCSRA, ADPS0); // ADC pre-scalar0 set division by 8
  SET(ADCSRA, ADPS1); // ADC pre-scalar1 set division by 8
  CLR(ADCSRA, ADPS2); // ADC pre-scalar2 cleared division by 8
  SET(ADMUX, REFS0); // REFS0 (bit 6) on for AREF <--> AVCC
  SET(ADCSRA, ADIE); // ADC interrupt enable
  SET(ADCSRA, ADEN); // ADC enable
  SET(ADCSRA, ADSC); // start adc conversion
}

static void display_logo(void)
{
  size_t i;
  for (i = 0; i < 130; i++) {
    //D
    write_sev_seg(0xFC, 0);
    //IM
    write_sev_seg(0xD8, 1);
    write_sev_seg(0xF0, 2);
    //P
    write_sev_seg(0xEA, 3);
  }
}

//static void self_test(void)
//{
//  size_t i;
//  size_t j;
//
//  for (i = 0; i < 10; i++)
//  {
//    volt_digits[0] = i;
//    volt_digits[1] = i;
//    volt_digits[2] = i;
//    volt_digits[3] = i;
//
//    for (j = 0; j < 30; j++)
//      display_digits(i % 3);
//    delay_ms(50);
//  }
//}

/////

int main(void)
{
  port_init();
  adc_init();
  sei();
  //self_test();
  display_logo();

  while (1)
  {
    SET(ADCSRA, ADSC); // start adc conversion
    if (adc_count == ADC_SAVE) // accumulate ADC readings
    {
      adc_count = 0; // reset the counter to zero
      adc2avg(); // average analog voltage sensor values
      adc2volt(); // convert analog readings into voltage fahrenheit
      digit_breakup(); // break voltage into hundreds, tens and ones places
    }
    display_digits(decimal_digit); // update 7-segment display w/ voltage
  }
  return(0);
}
