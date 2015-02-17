#include "outdrivers.h"
#include "notefinder.h"
#include <stdio.h>
#include <string.h>
#include "parameters.h"
#include <stdlib.h>
#include "color.h"
#include <math.h>
#include <unistd.h>
// #include <file.h>

struct SerialOutDriver
{
	FILE *device;
	FILE *triggerDevice;
	int did_init;
	int zigzag;
	int total_leds;
	int array;
	float outamp;
	uint8_t * last_leds;
	volatile int readyFlag;
	int xn;
	int yn;
	int rot90;
};


static void * SerialOutThread( void * v )
{
	struct SerialOutDriver * led = (struct SerialOutDriver*)v;
	while(1)
	{
		if( led->readyFlag && led->did_init )
		{
			fgetc(led->triggerDevice);
			size_t r = fwrite(led->last_leds, sizeof(uint8_t), (led->total_leds*3), led->device);
			fflush(led->device);
			if( r < 0 )
			{
				led->did_init = 0;
				printf( "Fault sending LEDs.\n" );
			}
			led->readyFlag = 0;
		}
		usleep(100);
	}
	return 0;
}

static void UpdateDeviceName(void* v) {
	struct SerialOutDriver * led = (struct SerialOutDriver*)v;

	led->readyFlag = 0;
	const char* file = GetParameterS("device", "/dev/ttyACM0");

	if(led->device) fclose(led->device);
	led->device = fopen(file, "w");
	if(led->triggerDevice) fclose(led->device);
	led->triggerDevice = fopen(file, "r");
	fprintf(stderr, "Devices ready%s\n", file);

	if( !led->device )
	{
		fprintf( stderr,  "Error: Cannot find device.\n" );
//			exit( -98 );
	}
	led->did_init = 1;
}

static void SerialUpdate(void * id, struct NoteFinder*nf)
{
	int i;
	struct SerialOutDriver * led = (struct SerialOutDriver*)id;

	while( led->readyFlag ) usleep(100);

	//Advance the LEDs to this position when outputting the values.
	for( i = 0; i < led->total_leds; i++ )
	{
		int source = i;
		if( !led->array )
		{
			int sx, sy;
			if( led->rot90 )
			{
				sy = i % led->yn;
				sx = i / led->yn;
			}
			else
			{
				sx = i % led->xn;
				sy = i / led->xn;
			}

			if( led->zigzag )
			{
				if( led->rot90 )
				{
					if( sx & 1 )
					{
						sy = led->yn - sy - 1;
					}
				}
				else
				{
					if( sy & 1 )
					{
						sx = led->xn - sx - 1;
					}
				}
			}

			if( led->rot90 )
			{
				source = sx + sy * led->xn;
			}
			else
			{
				source = sx + sy * led->yn;
			}
		}
		led->last_leds[i*3+0] = OutLEDs[source*3+1] * led->outamp;
		led->last_leds[i*3+1] = OutLEDs[source*3+0] * led->outamp;
		led->last_leds[i*3+2] = OutLEDs[source*3+2] * led->outamp;
	}
	fprintf(stderr, "Ready");
	led->readyFlag = 1;
}

static void SerialParams(void * id )
{
	struct SerialOutDriver * led = (struct SerialOutDriver*)id;

	led->total_leds = GetParameterI( "leds", 30 );
	led->last_leds = malloc( led->total_leds * 3 + 1 );
	led->outamp = 1.5; RegisterValue(  "ledoutamp", PAFLOAT, &led->outamp, sizeof( led->outamp ) );
	led->zigzag = 0; RegisterValue(  "zigzag", PAINT, &led->zigzag, sizeof( led->zigzag ) );
	led->xn = 16;		RegisterValue(  "lightx", PAINT, &led->xn, sizeof( led->xn ) );
	led->yn = 9;		RegisterValue(  "lighty", PAINT, &led->yn, sizeof( led->yn ) );
	led->rot90 = 0;	RegisterValue(  "rot90", PAINT, &led->rot90, sizeof( led->rot90 ) );
	led->array = 0;	RegisterValue(  "ledarray", PAINT, &led->array, sizeof( led->array ) );

	AddCallback("device", UpdateDeviceName, led);

	UpdateDeviceName(led);

	led->did_init = 0;
}


static struct DriverInstances * DisplaySerial()
{
	struct DriverInstances * ret = malloc( sizeof( struct DriverInstances ) );
	memset( ret, 0, sizeof( struct DriverInstances ) );
	struct SerialOutDriver * led = ret->id = malloc( sizeof( struct SerialOutDriver ) );
	ret->Func = SerialUpdate;
	ret->Params = SerialParams;
	OGCreateThread( SerialOutThread, led );
	led->readyFlag = 0;
	SerialParams( led );
	return ret;

}

REGISTER_OUT_DRIVER(DisplaySerial);
