#pragma once

enum NodeType
{
    NodeTypeDustSensor = 0,
    NodeTypeWindSensor = 1
};

// 1 -> overwrite settings stored in nvs
#define OVERRIDE_NODE_SETTINGS 0

// node is connected to a windsenser
#define NODE_TYPE NodeTypeDustSensor

// 0 -> is master
#define NODE_ID 0
