#ifndef _AVERAGE_RULE_H
#define _AVERAGE_RULE_H
/*
 * FogLAMP Average class
 *
 * Copyright (c) 2019 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */
#include <plugin.h>
#include <plugin_manager.h>
#include <config_category.h>
#include <rule_plugin.h>
#include <builtin_rule.h>
#include <map>
#include <string>

#define	MIN(x, y)	((x) < (y) ? (x) : (y))
/**
 * Average class, derived from Notification BuiltinRule
 */
class AverageRule: public BuiltinRule
{
	public:
		AverageRule();
		~AverageRule();

		void	configure(const ConfigCategory& config);
		void	lockConfig() { m_configMutex.lock(); };
		void	unlockConfig() { m_configMutex.unlock(); };
		bool	evaluate(const std::string& asset, const std::string& datapoint, long value);
		bool	evaluate(const std::string& asset, const std::string& datapoint, double value);
		typedef enum { SMA, EMA } AverageType;

	private:
		std::mutex	m_configMutex;
		long		m_deviation;
		std::string	m_direction;
		AverageType	m_aveType;
		int		m_factor;
		class Averages
		{
			public:
				Averages() : m_average(0), m_samples(0) {};
				~Averages();
				void	addValue(double value)
					{
						m_samples++;
						int divisor = m_type == AverageRule::SMA ? m_samples
							: MIN(m_samples, m_factor);
						m_average += ((value - m_average) / m_samples);
					};
				double	average() { return m_average; };
				void	setAverageType(AverageRule::AverageType type, int factor)
					{
						m_type = type;
						m_factor = factor;
					}
			private:
				double	m_average;
				int	m_samples;
				int	m_factor;
				AverageRule::AverageType
					m_type;
		};
		std::map<std::string, Averages *>
				m_averages;
};

#endif
