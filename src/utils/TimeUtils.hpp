/**
 * TimeUtils for handling time related stuff.
 * 
 * @file TimeUtils.cpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_TIME_UTILS_HPP
#define SPI_TIME_UTILS_HPP

#include <chrono>
#include <string>

namespace spi {

/**
 * Type for representing a point in time (timestamp)
 */
typedef std::chrono::time_point<std::chrono::system_clock> TimeStamp;


/**
 * Comparator for TimeStamps that is needed e.g. for maps
 */
struct TimeStampComparator {

    bool operator()(const TimeStamp &ts1, const TimeStamp &ts2) const {
        return ts1 < ts2;
    }

};


class TimeUtils {
public:

    /**
     * Constant representing the zero timestamp
     */
    static inline TimeStamp ZERO = std::chrono::time_point<std::chrono::system_clock>(std::chrono::microseconds(0));

    /**
     * Returns a timestamp representing the current point in time.
     * 
     * @return TimeStamp representing current point in time.
     */
    static TimeStamp now(){
        return std::chrono::system_clock::now();
    }

    /**
     * Returns a human readable string of the current point in time.
     * 
     * @return std::string String readable for humans that represents current point in time.
     */
    static std::string nowString(){
        return toString(now());
    }

    /**
     * Checks if a given timestamp is in the future.
     * 
     * @param timestamp TimeStamp to check.
     * @return true if timestamp is in the future, false otherwise.
     */
    static bool isFuture(const TimeStamp &timestamp){
        return timestamp > now();
    }

    /**
     * Checks if a given timestamp is in the past.
     * 
     * @param timestamp TimeStamp to check.
     * @return true if timestamp is in the past, false otherwise.
     */
    static bool isPast(const TimeStamp &timestamp){
        return timestamp < now();
    }

    /**
     * Checks if a given timestamp is zero.
     * 
     * @param timestamp TimeStamp to check.
     * @return true if timestamp is zero.
     */
    static bool isZero(const TimeStamp &timestamp){
        return timestamp == ZERO;
    }


    /**
     * Compares if two timestamps are equal at microsecond precision.
     * 
     * @param ts1 TimeStamp 1.
     * @param ts2 TimeStamp 2.
     * @return true if both represent same point in time at microsecond precision, false otherwise.
     */
    static bool equal(const TimeStamp &ts1, const TimeStamp &ts2){
        return toUInt64(ts1) == toUInt64(ts2);
    }


    /**
     * Adds a given duration in seconds to a given TimeStamp.
     * 
     * @param timestamp TimeStamp to which given duration should be added.
     * @param seconds Seconds that should be added to TimeStamp.
     * @return TimeStamp New TimeStamp with given duration added to it.
     */
    static TimeStamp addSeconds(TimeStamp timestamp, int64_t seconds){
        return timestamp + std::chrono::seconds(seconds);
    }

    /**
     * Adds a given duration in milliseconds to a given TimeStamp.
     * 
     * @param timestamp TimeStamp to which given duration should be added.
     * @param milliseconds Milliseconds that should be added to TimeStamp.
     * @return TimeStamp New TimeStamp with given duration added to it.
     */
    static TimeStamp addMilliseconds(TimeStamp timestamp, int64_t milliseconds){
        return timestamp + std::chrono::milliseconds(milliseconds);
    }

    /**
     * Adds a given duration in microseconds to a given TimeStamp.
     * 
     * @param timestamp TimeStamp to which given duration should be added.
     * @param microseconds Microseconds that should be added to TimeStamp.
     * @return TimeStamp New TimeStamp with given duration added to it.
     */
    static TimeStamp addMicroseconds(TimeStamp timestamp, int64_t microseconds){
        return timestamp + std::chrono::microseconds(microseconds);
    }


    /**
     * Returns the difference between two timestamps in seconds.
     * 
     * @param from Earlier timestamp (gets subtracted from 'until').
     * @param until Later timestamp (closer to now).
     * @return int64_t Difference in seconds (if 'from' before 'until' then value positive).
     */
    static int64_t differenceInSeconds(const TimeStamp &from, const TimeStamp &until){
        return std::chrono::duration_cast<std::chrono::seconds>(until - from).count();
    }

    /**
     * Returns the difference between a given timestamp and now in seconds (now - from).
     * 
     * @param from Earlier timestamp (gets subtracted from now).
     * @return int64_t Difference in seconds (if 'from' before now then value positive).
     */
    static int64_t differenceInSeconds(const TimeStamp &from){
        return differenceInSeconds(from, now());
    }

    /**
     * Returns the difference between two timestamps in milliseconds.
     * 
     * @param from Earlier timestamp (gets subtracted from 'until').
     * @param until Later timestamp (closer to now).
     * @return int64_t Difference in milliseconds (if 'from' before 'until' then value positive).
     */
    static int64_t differenceInMilliseconds(const TimeStamp &from, const TimeStamp &until){
        return std::chrono::duration_cast<std::chrono::milliseconds>(until - from).count();
    }

    /**
     * Returns the difference between a given timestamp and now in milliseconds (now - from).
     * 
     * @param from Earlier timestamp (gets subtracted from now).
     * @return int64_t Difference in milliseconds (if 'from' before now then value positive).
     */
    static int64_t differenceInMilliseconds(const TimeStamp &from){
        return differenceInMilliseconds(from, now());
    }

    /**
     * Returns the difference between two timestamps in microseoncds.
     * 
     * @param from Earlier timestamp (gets subtracted from 'until').
     * @param until Later timestamp (closer to now).
     * @return int64_t Difference in microseconds (if 'from' before 'until' then value positive).
     */
    static int64_t differenceInMicroseconds(const TimeStamp &from, const TimeStamp &until){
        return std::chrono::duration_cast<std::chrono::microseconds>(until - from).count();
    }

    /**
     * Returns the difference between a given timestamp and now in microseconds (now - from).
     * 
     * @param from Earlier timestamp (gets subtracted from now).
     * @return int64_t Difference in microseconds (if 'from' before now then value positive).
     */
    static int64_t differenceInMicroseconds(const TimeStamp &from){
        return differenceInMicroseconds(from, now());
    }

    /**
     * Converts a TimeStamp to UTC microseconds.
     * 
     * @param timestamp TimeStamp to convert.
     * @return uint64_t Integer representing TimeStamp.
     */
    static uint64_t toUInt64(const TimeStamp &timestamp){
        auto epoch = std::chrono::time_point_cast<std::chrono::microseconds>(timestamp).time_since_epoch();
        return std::chrono::duration_cast<std::chrono::microseconds>(epoch).count();
    }

    /**
     * Converts an integer to a TimeStamp.
     * 
     * @param value Integer representing a TimeStamp.
     * @return TimeStamp interpreted from given integer.
     */
    static TimeStamp fromUInt64(uint64_t value){
        return std::chrono::time_point<std::chrono::system_clock>(std::chrono::microseconds(value));
    }

    /**
     * Converts a given TimeStamp to a human readable string.
     * 
     * @param timestamp TimeStamp to convert.
     * @return std::string String readable for humans that represents timestamp.
     */
    static std::string toString(const TimeStamp &timestamp){
        auto time = std::chrono::system_clock::to_time_t(timestamp);
        std::string str = std::string(std::ctime(&time));
        return str.substr(0, str.length() - 1);
    }

};


}
#endif // SPI_TIME_UTILS_HPP