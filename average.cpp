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
#include <logger.h>
#include <plugin_exception.h>
#include <iostream>
#include <config_category.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <builtin_rule.h>
#include "version.h"
#include "average.h"

using namespace std;

/**
 * Average rule constructor
 *
 * Call parent class BuiltinRule constructor
 */
AverageRule::AverageRule() : BuiltinRule()
{
}

/**
 * Average destructor
 */
AverageRule::~AverageRule()
{
}

/**
 * Configure the rule plugin
 *
 * @param    config	The configuration object to process
 */
void AverageRule::configure(const ConfigCategory& config)
{
	// Remove current triggers
	// Configuration change is protected by a lock
	lockConfig();
	if (hasTriggers())
	{       
		removeTriggers();
	}       
	// Release lock
	unlockConfig();

	string assetName = config.getValue("asset");
	addTrigger(assetName, NULL);
	m_deviation = strtol(config.getValue("deviation").c_str(), NULL, 10);
	m_direction = config.getValue("direction");
	string aveType = config.getValue("averageType");
	if (aveType.compare("Simple Moving Average") == 0)
	{
		m_aveType = SMA;
	}
	else
	{
		m_aveType = EMA;
	}
	m_factor = strtol(config.getValue("factor").c_str(), NULL, 10);
	for (auto it = m_averages.begin(); it != m_averages.end(); it++)
	{
		it->second->setAverageType(m_aveType, m_factor);
	}
}

/**
 * Evaluate a long value to see if the alert should trigger.
 * This will also build the value of the average reading using this
 * value.
 *
 * @param asset		The asset name we are processing
 * @param datapoint	The data point we are processing
 * @param value		The value to consider
 * @return true if the value exceeds the defined percentage deviation
 */
bool AverageRule::evaluate(const string& asset, const string& datapoint, long value)
{
	return evaluate(asset, datapoint, (double)value);
}


/**
 * Evaluate a double value to see if the alert should trigger.
 * This will also build the value of the average reading using this
 * value.
 *
 * @param asset		The asset name we are processing
 * @param datapoint	The data point we are processing
 * @param value		The value to consider
 * @return true if the value exceeds the defined percentage deviation
 */
bool AverageRule::evaluate(const string& asset, const string& datapoint, double value)
{
	map<string, Averages *>::iterator it;

	if ((it = m_averages.find(datapoint)) == m_averages.end())
	{
		Averages *ave = new Averages();
		ave->addValue(value);
		ave->setAverageType(m_aveType, m_factor);
		m_averages.insert(pair<string, Averages *>(datapoint, ave));
		return false;
	}
	double average = it->second->average();
	it->second->addValue(value);
	double deviation = ((value - average) * 100) /average;
	bool rval = false;
	if (m_direction.compare("Both") == 0)
	{
		rval = fabs(deviation) > m_deviation;
	}
	else if (m_direction.compare("Above Average") == 0)
	{
		rval = deviation > m_deviation;
	}
	else if (m_direction.compare("Below Average") == 0)
	{
		rval = -deviation > m_deviation;
	}
	if (rval)
	{
		Logger::getLogger()->warn("Deviation of %.1f%% in %s.%s  triggered alert, value is %.2f, average is %.2f",
					  deviation,
					  asset.c_str(),
					  datapoint.c_str(),
					  value,
					  average);
	}
	return rval;
}
