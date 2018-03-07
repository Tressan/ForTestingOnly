/// \file SPI_On_Uno_Example_With_Interrupt.ino
///		\brief Sample of SPI slave and master using interrupts.
///
/// Pins
///
/// The SPI hardware on the AVR chip is connected to these ICSP pins.
/// On the Uno, these pins are also connected to D11, D12, D13. 
/// ICSP pins:              <br>
//
/// MISO &#09; is ICSP 1 			<br>
/// SCK &#09; is ICSP 3				<br>
/// MOSI &#09; is ICSP 4			<br>
///

#define _SS 10
#define BUFFER_SIZE 32


uint8_t masterMessage[BUFFER_SIZE+1] =   "|--- Master Uno / interrupt ---|";
uint8_t slaveMessage[BUFFER_SIZE+1] =    "|---- Slave Uno / interrupt ---|";
uint8_t inputBuffer[BUFFER_SIZE];
uint8_t outputBuffer[BUFFER_SIZE];

uint16_t inputBufferCounter;
uint16_t outputBufferCounter;

bool iAmMaster;
bool iAmSlave;

void setup()
{
	iAmMaster = false;
	iAmSlave = false;
	Serial.begin ( 115200 );
	while ( !Serial ) ; // wait for port to open
	showMenu();	
}

void loop()
{
	loopSerial();
	loopSPI();
}

/// Wait for a user command in the Serial Monitor.
void loopSerial()
{
	// If user enters a command, do it.
	
	if ( Serial.available() )
	{
		uint8_t clu = Serial.read();
		if ( 0x20 <= clu < 0xFF )
			Serial.println((char)clu);
		switch ( clu )
		{
			case 'S':
				if ( ! iAmMaster )
				{
					if ( iAmSlave )
					{
						SPCR = 0;
						Serial.println ( "stop listening" );
						iAmSlave = false;
					}
					else
					{
						startSlave();
					}
				}
				break;
			case 'M':
				startMaster();
				break;
		}
		showMenu();	
	}
}

/// Check for full input buffer.
void loopSPI()
{
	if ( inputBufferCounter >= BUFFER_SIZE )
	{
		reportInputBuffer();
		outputBufferCounter = 0;
		if ( iAmSlave )
			startSlave();	
		else
			stopMaster();	
	}
}

void stopMaster()
{
	iAmMaster = false;
	SPCR = 0;
	digitalWrite ( _SS, LOW );
}


/// Start to listen as slave until commanded to stop listening.
void startSlave()
{
	for ( outputBufferCounter = 0; outputBufferCounter < BUFFER_SIZE; outputBufferCounter++ )
		outputBuffer[outputBufferCounter] = slaveMessage[outputBufferCounter];

	// Implicitly data order is msb first and
	// Clock is low when idle and
	// clock sample on leading edge, setup on trailing
	
	pinMode ( _SS, INPUT );
	pinMode ( MISO, OUTPUT );
//	oldSelected = digitalRead ( _SS ) == LOW;

	// Enable the SPI, without master and interrupt bits
	// Implicitly data order is msb first and
	// Clock is low when idle and
	// clock sample on leading edge, setup on trailing
	// For details about the Spi Control Register, consult the datasheet for AVR processors.
	SPCR =  1 << SPE | 1 << SPIE;

	outputBufferCounter = 0;
	inputBufferCounter = 0;
	
	SPDR = slaveMessage[outputBufferCounter++];
	iAmSlave = true;
	Serial.println ( "Listening as slave" );
}

/// Send and receive one buffer as master and then stop
void startMaster()
{
	Serial.println ( "Start Master" );
	iAmMaster = true;
	
	for ( outputBufferCounter = 0; outputBufferCounter < BUFFER_SIZE; outputBufferCounter++ )
		outputBuffer[outputBufferCounter] = masterMessage[outputBufferCounter];
	pinMode ( _SS, OUTPUT );
	pinMode ( MISO, INPUT );
//	pinMode ( MOSI, OUTPUT );
//	pinMode ( SCK, OUTPUT );
	
	// Enable the SPI in master mode.
	digitalWrite ( _SS, LOW );
	outputBufferCounter = 0;
	inputBufferCounter = 0;
	SPCR = 1 << SPE | 1 << SPIE | 1 << MSTR | 1 << SPR0 | 1 << SPR1;	
	SPDR = outputBuffer[outputBufferCounter++];
}

void reportInputBuffer()
{
	uint16_t count;
	Serial.println("---");
	Serial.print (F( "Input buffer count=" ));
	Serial.println ( inputBufferCounter );
	Serial.print (F( "Output buffer count=" ));
	Serial.println ( outputBufferCounter );
	Serial.print (F( " Buffer contents: " ));
	Serial.write ( inputBuffer, inputBufferCounter );
	Serial.println();
	
	for ( count = 0; count < inputBufferCounter; count++ )
	{
		Serial.print ( " " );
		Serial.print ( inputBuffer[count], HEX );
	}
	Serial.println ();
	Serial.println ("---");
	
	for ( count = 0; count < inputBufferCounter; count++ );
		inputBuffer[count] = '?';
	inputBufferCounter = 0;
	outputBufferCounter = 0;
}


/// SPI interrupt routine. Transmit and receive one byte. 
ISR (SPI_STC_vect)
{
	uint8_t clu = SPDR;
  	if ( inputBufferCounter < BUFFER_SIZE )
		inputBuffer[inputBufferCounter++]=clu;
	if ( outputBufferCounter < BUFFER_SIZE )
		SPDR = outputBuffer[outputBufferCounter++];
}  

void showMenu()
{
	Serial.println ( F( "SPI_On_Uno_Example_With_Interrupt.ino") );	
	if ( iAmSlave )
	{
		Serial.println ( F( "Listening in slave mode") );
		Serial.println ( F( "S:Stop slave mode" ) );
	}
	else
	{
		Serial.println ( F("S:Start listening in slave mode") );
		Serial.println ( F("M:Send a message in master mode") );	
	}	
}
