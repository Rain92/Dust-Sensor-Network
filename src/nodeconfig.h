#pragma once

enum NodeType
{
    NodeTypeDustSensor = 0,
    NodeTypeWindSensor = 1
};

// 1 -> overwrite settings stored in nvs
// 0 -> use settings stored in nvs if aviable
#define OVERRIDE_NODE_SETTINGS 0

// node type; dustsensor or windsenser
#define NODE_TYPE NodeTypeDustSensor

// ID of the node; 0 -> master
#define NODE_ID 0

// if nonzero the master will try to publish data to the specified mqtt server
#define USE_MQTT 0

// if nonzero the touch pin of the master node can be used
// to start/stop the measuring process
#define USE_TOUCH_CONTROL 0

// if nonzero display battery voltage
#define USE_BATTERY 0