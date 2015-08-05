/*
   MySensor unified classes for MyMeteo project
   Author: Odinchenko Aleksey (aleksey.clx@gmail.com)
   Last changes: 2015-08-06
*/

#ifndef __MYSENSOR_H__
#define __MYSENSOR_H__

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// ADAFRUIT unified compatible
typedef enum {
 none                =(0),
 Aeecelrometer       =(1),
 MagneticField       =(2),
 Orientation         =(3),
 Gyroscope           =(4),
 Light               =(5),
 Pressure            =(6),
 Proximity           =(7),
 Gravity             =(8),
 Acceleration	     =(10),
 RotationVector      =(11),
 Humidity            =(12),
 Temperature         =(13),
 Voltage             =(15),
 Current             =(16),
 Color               =(17),
 Other               =(255)
} mysensor_value_kind_e;

#define SENS_VALUE_KIND_NONE                0x00
#define SENS_VALUE_KIND_ACCELEROMETER       0x01
#define SENS_VALUE_KIND_MAGNETIC_FIELD      0x02
#define SENS_VALUE_KIND_ORIENTATION         0x03
#define SENS_VALUE_KIND_GYROSCOPE           0x04
#define SENS_VALUE_KIND_LIGHT               0x05
#define SENS_VALUE_KIND_PRESSURE            0x06
#define SENS_VALUE_KIND_PROXIMITY           0x07
#define SENS_VALUE_KIND_GRAVITY             0x08
#define SENS_VALUE_KIND_LINEAR_ACCELERATION 0x09
#define SENS_VALUE_KIND_ROTATION_VECTOR     0x0A
#define SENS_VALUE_KIND_RELATIVE_HUMIDITY   0x0B
#define SENS_VALUE_KIND_AMBIENT_TEMPERATURE 0x0C
#define SENS_VALUE_KIND_VOLTAGE             0x0D
#define SENS_VALUE_KIND_CURRENT             0x0E
#define SENS_VALUE_KIND_COLOR               0x0F
#define SENS_VALUE_KIND_HEAT_INDEX          0x10
#define SENS_VALUE_KIND_OTHER               0xFF

#define INTERVAL_SCAN_LOCAL_SEC 10
#define INTERVAL_SCAN_REMOTE_SEC 60

/*
*    device(id)
*      <--- sensor(src,id)
*          <- value1 (kind,id)
*          <- value2 (kind,id)
*
*/

#define MYSENSOR_LIST_CAPACITY 2
#define MYSENSOR_VALUES_CAPACITY 5

// unified sensor read value (4bytes)
typedef union {
	float asFloat;
	uint32_t asUI32;
	struct {
		uint16_t x;
		uint16_t y;
	} asPoint;
	uint8_t asBytes[4];
} mysensor_value_u;


class MySensorValue {
	public:
		uint8_t id;     // sensor value id
		uint8_t kind;   // values kind SENS_VALUE_KIND... (for one id can be few kind)
		mysensor_value_u value;
	
		// todo : max, min
	
		MySensorValue();

		void printInfo();
		const char* kindName();
		const char* kindMetric();
	//void printValueUnion(mysensor_value_u *v);
};


/*
class MySensorValueList {
	public:
		MySensorValue items[MYSENSOR_VALUE_LIST_CAPACITY];
		uint8_t count;

		MySensorValueList();
};
*/


class MySensor {
	public:
		char* name;                 // Sensor group name: e.g: "Local", "Remote"
		uint16_t owner;             // owner device id (mask for Master sensors: 0xFFFF0000, for Slave sensors: 0x0000FFFF)
		uint8_t local_id;           // sensor local_id for device (1...0xFF)
	
		MySensorValue values[MYSENSOR_VALUES_CAPACITY];
		uint8_t values_count;

		// void (*on_init)(MySensor* s);              // sensor initialization
		uint8_t (*on_scan)(MySensor* s, bool overrideChanged); // on sensor values scan
	
		uint16_t scan_interval_sec; // auto-scan interval time ( 0= local, PipeN for remote), set when sensor created
		uint32_t last_tick_scan;    // last sensor scan
		uint8_t just_scaned;        // dynamic: flag: reset every cycle and set if values received

		MySensor();
		void init(uint16_t owner, uint8_t id, char* name, uint16_t scanIntervalSec); //, void (*onInit)(MySensor *s), uint8_t (*onScan)(MySensor *s, bool everride));
		void valuesClear();
	
		bool timeToScan(uint32_t* tick);
		MySensorValue* getValue(uint8_t vId, uint8_t vKind, bool addIfNone);
	
		bool setValue(uint8_t id, uint8_t kind, const mysensor_value_u *value);
		bool setValueF(uint8_t id, uint8_t kind, float value, float onlyDeltaAbove);
		//bool setValueUI32(uint8_t id, uint8_t kind, uint32_t value, uint32_t onlyDelta);
		
		void printName(bool printOwner, const char* appendText);
		void printValues(bool printName, bool printId, const char* nameAppendText);

		uint8_t scan(bool overrideChanged);
		uint8_t scanOnDemand(uint32_t *tick, bool overrideScan, bool overrideScanged);
};

class MySensorList {
	public:
		MySensor items[MYSENSOR_LIST_CAPACITY];
		uint8_t count;

		MySensorList();
		MySensor* get(uint16_t owner, uint8_t id, bool addIfNone);
};


extern const char* _value_kind_names[];
extern const char* _value_kind_metrics[];
extern const char* mysensor_value_name(uint8_t kind);
extern const char* mysensor_value_metric(uint8_t kind);

#endif
