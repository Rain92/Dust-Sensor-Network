#pragma once

enum NodeType
{
    NodeTypeDustSensor = 0,
    NodeTypeWindSensor = 1
};

// 1 -> overwrite settings stored in nvs
// 0 -> use settings stored in nvs if aviable
#define OVERRIDE_NODE_SETTINGS 0

// node is connected to a windsenser?
#define NODE_TYPE NodeTypeDustSensor

// 0 -> is master
#define NODE_ID 0

// if nonzero the master will try to publish data to the specified mqtt server
#define USE_MQTT 1