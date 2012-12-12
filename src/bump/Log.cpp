//
//	Log.cpp
//	Bump
//
//	Created by Christian Noon on 12/3/12.
//	Copyright (c) 2012 Christian Noon. All rights reserved.
//

// C++ headers
#include <fstream>

// Boost headers
#include <boost/date_time/posix_time/posix_time.hpp>

// Bump headers
#include <bump/Environment.h>
#include <bump/Log.h>

using namespace bump;

Log::Log() :
	_isEnabled(true),
	_logLevel(WARNING_LVL),
	_isDateTimeFormatEnabled(false),
	_dateTimeFormat(DATE_TIME_DEFAULT),
	_logStream(&std::cout)
{
	// Attempt to disable the entire log system based on the "BUMP_LOG_ENABLED" environment variable
	String logEnabled = bump::Environment::environmentVariable(BUMP_LOG_ENABLED);
	logEnabled.toLowerCase();
	if (logEnabled == "no" || logEnabled == "false" || logEnabled == "nope" || logEnabled == "disable")
	{
		_isEnabled = false;
		std::cout << "[bump] Setting LOG_ENABLED to NO" << std::endl;
		return;
	}

	// Attempt to set the log level based on the "BUMP_LOG_LEVEL" environment variable
	String logLevel = bump::Environment::environmentVariable(BUMP_LOG_LEVEL);
	if (logLevel == "ALWAYS_LVL")
	{
		_logLevel = ALWAYS_LVL;
		std::cout << "[bump] Setting BUMP_LOG_LEVEL to ALWAYS" << std::endl;
	}
	else if (logLevel == "ERROR_LVL")
	{
		_logLevel = ERROR_LVL;
		std::cout << "[bump] Setting BUMP_LOG_LEVEL to ERROR" << std::endl;
	}
	else if (logLevel == "WARNING_LVL")
	{
		_logLevel = WARNING_LVL;
		std::cout << "[bump] Setting BUMP_LOG_LEVEL to WARNING" << std::endl;
	}
	else if (logLevel == "INFO_LVL")
	{
		_logLevel = INFO_LVL;
		std::cout << "[bump] Setting BUMP_LOG_LEVEL to INFO" << std::endl;
	}
	else if (logLevel == "DEBUG_LVL")
	{
		_logLevel = DEBUG_LVL;
		std::cout << "[bump] Setting BUMP_LOG_LEVEL to DEBUG" << std::endl;
	}
	else if (!logLevel.empty())
	{
		std::cout << "[bump] WARNING: Your BUMP_LOG_LEVEL environment variable: [" << logLevel
			<< "] does not match any of the possible options: [ ALWAYS_LVL | ERROR_LVL | WARNING_LVL "
			<< "| INFO_LVL | DEBUG_LVL ]" << std::endl;
	}
	else
	{
		// DO NOTHING
	}

	// Attempt to set the log file based on the "BUMP_LOG_FILE" environment variable
	String logFile = bump::Environment::environmentVariable(BUMP_LOG_FILE);
	if (!logFile.empty())
	{
		if (logFile == "stderr")
		{
			_logStream = &std::cerr;
		}
		else if (logFile != "stdout")
		{
			bool success = setLogFile(logFile);
			if (success)
			{
				std::cout << "[bump] Setting BUMP_LOG_FILE to " << logFile << std::endl;
			}
			else
			{
				std::cout << "[bump] WARNING: Your BUMP_LOG_FILE environment variable: ["
					<< logFile << "] could not be created or opened" << std::endl;
			}
		}
	}
}

Log::~Log()
{
	;
}

void Log::setIsLogEnabled(bool enabled)
{
	boost::mutex::scoped_lock lock(_mutex);
	_isEnabled = enabled;
}

bool Log::isLogEnabled()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _isEnabled;
}

bool Log::isLogLevelEnabled(LogLevel logLevel)
{
	boost::mutex::scoped_lock lock(_mutex);

	// Returning false if disabled
	if (!_isEnabled)
	{
		return false;
	}

	return logLevel <= _logLevel;
}

void Log::setLogLevel(LogLevel logLevel)
{
	boost::mutex::scoped_lock lock(_mutex);
	_logLevel = logLevel;
}

Log::LogLevel Log::logLevel()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _logLevel;
}

void Log::setDateTimeFormat(const DateTimeFormat& format)
{
	boost::mutex::scoped_lock lock(_mutex);
	_dateTimeFormat = format;
}

void Log::setIsDateTimeFormattingEnabled(bool enabled)
{
	boost::mutex::scoped_lock lock(_mutex);
	_isDateTimeFormatEnabled = enabled;
}

bool Log::isDateTimeFormattingEnabled()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _isDateTimeFormatEnabled;
}

Log::DateTimeFormat Log::dateTimeFormat()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _dateTimeFormat;
}

bool Log::setLogFile(const String& filepath)
{
	boost::mutex::scoped_lock lock(_mutex);

	// First try to open the file
	std::ofstream* logFile = new std::ofstream(filepath.c_str());
	if ( !(*logFile) )
	{
		delete logFile;
		logFile = NULL;
		return false;
	}

	// We successfully opened the file for writing, so switch log streams
	_logStream = logFile;
	return true;
}

void Log::setLogStream(std::ostream& stream)
{
	boost::mutex::scoped_lock lock(_mutex);
	_logStream = &stream;
}

std::ostream& Log::logStream(const String& prefix)
{
	boost::mutex::scoped_lock lock(_mutex);

	// The generic stream to "buffer" output to for the output handler. This allows us to
	// avoid pointer dereferences until necessary.
	std::ostream& ostream = *_logStream;

	// Append the date time if necessary
	if (_isDateTimeFormatEnabled)
	{
		ostream << _convertTimeToString() << " ";
	}

	// Append the prefix if necessary
	if (!prefix.empty())
	{
		ostream << prefix;
	}

	return *_logStream;
}

String Log::_convertTimeToString()
{
	// Get the time using boost
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

	// Build strings for each time value from the boost ptime value
	String year = static_cast<long>(now.date().year());
	String month = static_cast<long>(now.date().month());
	String day = static_cast<long>(now.date().day());
	String hours = static_cast<long>(now.time_of_day().hours());
	String minutes = static_cast<long>(now.time_of_day().minutes());
	String seconds = static_cast<long>(now.time_of_day().seconds());

	// Figure out if we're AM or PM
	String am_pm = hours.toInt() < 13 ? "AM" : "PM";

	// Correct the hours if larger than 12
	if (hours.toInt() > 12)
	{
		hours = hours.toInt() - 12;
	}

	// Pad the minutes
	while (minutes.length() < 2)
	{
		minutes.append("0");
	}

	// Pad the seconds
	while (seconds.length() < 2)
	{
		seconds.append("0");
	}

	// Now create a string representation of the time based on the date/time format
	if (_dateTimeFormat == DATE_TIME_DEFAULT)
	{
		return String("%1-%2-%3 %4:%5:%6").arg(year, month, day, hours, minutes, seconds);
	}
	else if (_dateTimeFormat == DATE_TIME_WITH_AM_PM)
	{
		return String("%1-%2-%3 %4:%5:%6 %7").arg(year, month, day, hours, minutes, seconds, am_pm);
	}
	else if (_dateTimeFormat == TIME_DEFAULT)
	{
		return String("%1:%2:%3").arg(hours, minutes, seconds);
	}
	else // _dateTimeFormat == TIME_WITH_AM_PM
	{
		return String("%1:%2:%3 %4").arg(hours, minutes, seconds, am_pm);
	}
}
