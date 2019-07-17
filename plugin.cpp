/**
 * FogLAMP Average notification rule plugin
 *
 * Copyright (c) 2019 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */

#include <plugin_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <logger.h>
#include <plugin_exception.h>
#include <iostream>
#include <config_category.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <builtin_rule.h>
#include "version.h"
#include "average.h"

#define RULE_NAME "Average"
#define DEFAULT_TIME_INTERVAL	30


#define TO_STRING(...) DEFER(TO_STRING_)(__VA_ARGS__)
#define DEFER(x) x
#define TO_STRING_(...) #__VA_ARGS__
#define QUOTE(...) TO_STRING(__VA_ARGS__)

static const char *default_config = QUOTE({
	"description":	{
			"description": RULE_NAME,
			"type": "string",
			"default":  RULE_NAME,
			"readonly": "true"
			},
	"plugin": 	{
			"description": RULE_NAME,
			"type": "string",
			"default":  RULE_NAME,
			"readonly": "true"
			},
	"asset": 	{
			"description" : "Asset to monitor",
			"type" : "string",
			"default" : "",
			"displayName" : "Asset",
			"order": "1"
		       	},
	"deviation":	{
			"description": "Allowed percentage deviation from average",
			"type": "integer",
			"default": "10",
			"displayName":"Deviation %",
			"order":"2"
			},
	"direction":	{
			"description": "Trigger on direction of deviation",
			"type": "enumeration",
			"options" : [ "Above Average", "Below Average", "Both" ],
			"default": "Both",
			"displayName":"Direction",
			"order":"3"
			},
	"averageType":	{
			"description": "The type of average to calculate",
			"type": "enumeration",
			"options" : [ "Simple Moving Average", "Exponential Moving Average" ],
			"default": "Simple Moving Average",
			"displayName":"Average",
			"order":"4"
			},
	"factor":	{
			"description": "Exponential moving average factor",
			"type": "integer",
			"default": "10",
			"displayName":"EMA Factor",
			"order":"5"
			}
	});


using namespace std;

/**
 * The C plugin interface
 */
extern "C" {
/**
 * The C API rule information structure
 */
static PLUGIN_INFORMATION ruleInfo = {
	RULE_NAME,			// Name
	VERSION,			// Version
	0,				// Flags
	PLUGIN_TYPE_NOTIFICATION_RULE,	// Type
	"1.0.0",			// Interface version
	default_config			// Configuration
};

/**
 * Return the information about this plugin
 */
PLUGIN_INFORMATION *plugin_info()
{
	return &ruleInfo;
}

/**
 * Initialise rule objects based in configuration
 *
 * @param    config	The rule configuration category data.
 * @return		The rule handle.
 */
PLUGIN_HANDLE plugin_init(const ConfigCategory& config)
{
	
	AverageRule *handle = new AverageRule();
	handle->configure(config);

	return (PLUGIN_HANDLE)handle;
}

/**
 * Free rule resources
 */
void plugin_shutdown(PLUGIN_HANDLE handle)
{
	AverageRule *rule = (AverageRule *)handle;
	// Delete plugin handle
	delete rule;
}

/**
 * Return triggers JSON document
 *
 * @return	JSON string
 */
string plugin_triggers(PLUGIN_HANDLE handle)
{
	string ret;
	AverageRule *rule = (AverageRule *)handle;

	if (!rule)
	{
		ret = "{\"triggers\" : []}";
		return ret;
	}

	// Configuration fetch is protected by a lock
	rule->lockConfig();

	if (!rule->hasTriggers())
	{
		ret = "{\"triggers\" : []}";
		return ret;
	}

	ret = "{\"triggers\" : [ ";
	std::map<std::string, RuleTrigger *> triggers = rule->getTriggers();
	for (auto it = triggers.begin();
		  it != triggers.end();
		  ++it)
	{
		ret += "{ \"asset\"  : \"" + (*it).first + "\"";
		ret += " }";
		
		if (std::next(it, 1) != triggers.end())
		{
			ret += ", ";
		}
	}

	ret += " ] }";

	// Release lock
	rule->unlockConfig();

	return ret;
}

/**
 * Evaluate notification data received
 *
 * @param    assetValues	JSON string document
 *				with notification data.
 * @return			True if the rule was triggered,
 *				false otherwise.
 */
bool plugin_eval(PLUGIN_HANDLE handle,
		 const string& assetValues)
{
	Document doc;
	doc.Parse(assetValues.c_str());
	if (doc.HasParseError())
	{
		return false;
	}

	bool eval = false; 
	AverageRule *rule = (AverageRule *)handle;
	map<std::string, RuleTrigger *>& triggers = rule->getTriggers();

	// Iterate throgh all configured assets
	// If we have multiple asset the evaluation result is
	// TRUE only if all assets checks returned true
	for (auto t = triggers.begin(); t != triggers.end(); ++t)
	{
		string assetName = t->first;
		if (doc.HasMember(assetName.c_str()))
		{
			// Get all datapoints for assetName
			const Value& assetValue = doc[assetName.c_str()];

			for (Value::ConstMemberIterator itr = assetValue.MemberBegin();
					    itr != assetValue.MemberEnd(); ++itr)
			{
				if (itr->value.IsInt64())
				{
					eval |= rule->evaluate(assetName, itr->name.GetString(), itr->value.GetInt64());
				}
				else if (itr->value.IsDouble())
				{
					eval |= rule->evaluate(assetName, itr->name.GetString(), itr->value.GetDouble());
				}
			}
		}
	}

	// Set final state: true is any calls to evalaute() returned true
	rule->setState(eval);

	return eval;
}

/**
 * Return rule trigger reason: trigger or clear the notification. 
 *
 * @return	 A JSON string
 */
string plugin_reason(PLUGIN_HANDLE handle)
{
	AverageRule* rule = (AverageRule *)handle;

	string ret = "{ \"reason\": \"";
	ret += rule->getState() == AverageRule::StateTriggered ? "triggered" : "cleared";
	ret += "\" }";

	return ret;
}

/**
 * Call the reconfigure method in the plugin
 *
 * Not implemented yet
 *
 * @param    newConfig		The new configuration for the plugin
 */
void plugin_reconfigure(PLUGIN_HANDLE handle,
			const string& newConfig)
{

	AverageRule* rule = (AverageRule *)handle;
	ConfigCategory  config("new_outofbound", newConfig);
	rule->configure(config);
}

// End of extern "C"
};
