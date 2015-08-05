#include "MySensor.h"

const char* _value_kind_names[] {
	"Other",
	"Accelerometer",
	"Magnetic",
	"Orientation",
	"Gyro",
	"Light",
	"Pressure",
	"Proximity",
	"Gravity",
	"Acceleration",
	"Rotation",
	"Humidity",
	"Temperature",
	"Voltage",
	"Current",
	"Color",
	"HeatIndex",
"ALL"};
#define _value_kind_names_count sizeof(_value_kind_names)/sizeof(char *) //array size

const char* _value_kind_metrics[] = {
	"Other",
	"G",
	"Magnetic",
	"Degrees",
	"Gyro",
	"%",
	"mm Hg",
	"cm",
	"G",
	"G",
	"'",
	"%",
	"'C",
	"V",
	"A",
	"Gamma",
	"'C",
"ALL"};
#define _value_kind_metrics_count sizeof(_value_kind_metrics)/sizeof(char *) //array size

char* str_sensor_name_local = "LOCAL";
char* str_sensor_name_remote = "REMOTE";

const char* mysensor_value_name(uint8_t kind) {
	if (kind<_value_kind_names_count)
	return _value_kind_names[kind];
	else
	return _value_kind_names[0];
}

const char* mysensor_value_metric(uint8_t kind) {
	if (kind<_value_kind_names_count)
	return _value_kind_metrics[kind];
	else
	return _value_kind_metrics[0];
}

const void mysensor_value_union_print(mysensor_value_u v) {
	if (v.asFloat==NAN) // bad!
	Serial.print(F("---"));
	else
	Serial.print(v.asFloat);
	Serial.print(' ');
}

// MySensorClass MySensor;

//
//
//  MySensorValue
//
//

MySensorValue::MySensorValue() {
}

const char* MySensorValue::kindName(){
	return mysensor_value_name(kind);
}

const char* MySensorValue::kindMetric(){
	return mysensor_value_metric(kind);
}


void MySensorValue::printInfo() {
	Serial.print(F(" id=")); Serial.print(id); Serial.print(F("; "));
	Serial.print(F(" kind=")); Serial.print(kind); Serial.print(F("; "));
	Serial.print(mysensor_value_name(kind)); Serial.print(F(" = "));
	if (value.asFloat==NAN) {
		Serial.println(F("---"));
		} else {
		Serial.print(value.asFloat);
		Serial.print(" "); Serial.println(mysensor_value_metric(kind));
	}
}

/*
*
* MySensorValueList
*
*/

mysensor_value_u mysensor_value_u_delta(mysensor_value_u *v1, mysensor_value_u *v2){
	mysensor_value_u v;
	// todo: check types
	float f = v1->asFloat-v2->asFloat;
	memcpy(&v, &f, sizeof(mysensor_value_u));
	return  v;
}

/*
MySensorValueList::MySensorValueList() {
	
}
*/

/*
*
*  MySensor
*
*/

MySensor::MySensor() {
}

void MySensor::init(uint16_t owner, uint8_t id, char* name, uint16_t scanIntervalSec) {//, void (*onInit)(MySensor *s), uint8_t (*onScan)(MySensor *s, bool everride)) {
	// memset(s, 0, sizeof(mysensor_t));
	this->owner = owner;
	this->local_id = id;
	if (name) this->name = name;
	this->scan_interval_sec=scanIntervalSec;
	// Serial.println(scan_interval_sec*1000UL);
	this->values_count=0;
	//this->on_scan = onScan;
	//if (onInit) {
	//	onInit(this);
	//}
}


// return value is changed
// external update: last_tick_scan
uint8_t MySensor::scan(bool overrideChanged) {
	this->just_scaned=true;
	if (on_scan) return on_scan(this, overrideChanged);
	else return 0; // override?
}
 
 // scan if time to scan or override, return = 1 if scan and has changed
uint8_t MySensor::scanOnDemand(uint32_t *tick, bool overrideScan, bool overrideChanged) {
	just_scaned=false;
	if (overrideScan || timeToScan(tick)) {
		Serial.println(F("Scan sensor..."));
		last_tick_scan = *tick;
		// just_scaned = true;
		return scan(overrideChanged);
	}
}
 
void MySensor::valuesClear() {
	values_count=0;
	// todo: dispose objects?
}

//
//todo: scan_interval_sec = 0xFFFF for every tick scan ?
bool MySensor::timeToScan(uint32_t* tick) {
	unsigned long elapsed = *tick-last_tick_scan;
	//Serial.println(scan_interval_sec*1000UL);
	//Serial.println(elapsed);
	//delay(1000);
	if (!last_tick_scan || (scan_interval_sec>0 && elapsed>scan_interval_sec*1000UL))
	return true;
	else
	return false;
}

// todo: return pointer?
MySensorValue* MySensor::getValue(uint8_t id, uint8_t kind, bool addIfNone) {
	uint8_t i=0;
	while (i<values_count) {
		if (values[i].id==id && values[i].kind==kind) return &values[i];
		i++;
	}
	
	if (addIfNone) {
		if (i>=MYSENSOR_VALUES_CAPACITY) {
			Serial.print(F("Sensor values capacity is ")); Serial.println(MYSENSOR_VALUES_CAPACITY);
			return NULL;
		}

		MySensorValue *v=&values[i];
		// memset(v, 0, sizeof(MySensorValue));
		v->id = id;
		v->kind = kind;
		v->value.asFloat = NAN; // what is as buf ?
		
		values_count=i+1;
		return v;
	}
	return NULL;
}

bool MySensor::setValue(uint8_t id, uint8_t kind, const mysensor_value_u *value){
	MySensorValue *v = getValue(id, kind, true);
	if (v) {
		v->value = *value;
		return true;
	};
	return false;
}


bool MySensor::setValueF(uint8_t id, uint8_t kind, float value, float onlyDeltaAbove){
	MySensorValue *v = getValue(id, kind, true);
	if (v) {
		if (onlyDeltaAbove==0
		|| abs(v->value.asFloat-value)>=onlyDeltaAbove) {
			v->value.asFloat = value;
			return true;
		}
	};
	return false;
}


void MySensor::printName(bool printOwner, const char* appendText) {
	if (name)
	Serial.print(name);

	if (printOwner || !this->name) {
		Serial.print(" ("); Serial.print(this->owner);
		Serial.print("."); Serial.print(this->local_id); Serial.print(")");
	}
	if (appendText)
	Serial.print(appendText);
}


void MySensor::printValues(bool printName, bool printId, const char* nameAppendText) {
	if (printName)
	this->printName(printId, nameAppendText);
	
	// Serial.println(sensor.values.count);
	for (uint8_t i=0; i<values_count; i++) {
		values[i].printInfo();
	}
}


/*
*
* MySensorList
*
*/


MySensorList::MySensorList() {
}

MySensor* MySensorList::get(uint16_t owner, uint8_t id, bool addIfNone) {
	// Serial.print("sensorlist.count="); Serial.println(l->count);
	// Serial.print("Searching sensor: src="); Serial.print(src); Serial.print(" id="); Serial.println(id);
	MySensor *s;
	uint8_t i=0;
	while (i < this->count) {
		s = &items[i]; // Serial.print("  list sensor: src="); Serial.print(s->src); Serial.print(" id="); Serial.println(s->id);
		if (s->owner==owner && s->local_id==id)
		{
			return s;
		}
		i++;
	}
	
	if (addIfNone) {
		//Serial.print("adding at ");Serial.println(i);
		// Serial.print("sizeof(sensorlist.items)"); Serial.println(sizeof(l->items));
		//Serial.print("MaxCapaticy="); Serial.println(MYSENSOR_LIST_CAPACITY);
		if (i>=MYSENSOR_LIST_CAPACITY) {
			Serial.print(F("ERR: Sensor list capacity is ")); Serial.println(MYSENSOR_LIST_CAPACITY);
			return NULL;
		}
		
		s=&items[i];
		bool isLocal = owner>>12; // mask 0xF000

		s->init(owner, id,
		isLocal? str_sensor_name_local : str_sensor_name_remote,
		isLocal? INTERVAL_SCAN_LOCAL_SEC : INTERVAL_SCAN_REMOTE_SEC); // no name, no auto-scan interval
		
		Serial.print(F("New sensor: "));  s->printName(true, NULL); Serial.println();
		
		count=i+1;
		//Serial.print("new count=");Serial.println(l->count);
		return s;
	}
	return NULL;
}
