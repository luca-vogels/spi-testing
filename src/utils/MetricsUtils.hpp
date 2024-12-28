/**
 * MetricsUtils helps to handle different metrics and to convert them into human readable formats.
 * 
 * @file MetricsUtils.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_METRICS_UTILS_HPP
#define SPI_METRICS_UTILS_HPP

#include <cstdint>
#include <math.h>
#include <string>

namespace spi {



enum class ByteSizeMetric : uint16_t {
    KILOBYTE = 1000,
    KIBIBYTE = 1024,
};


class MetricsUtils {


public:

    /**
     * Rounds a given value and returns it as string.
     * 
     * @param value Value to round
     * @return std::string Value as string.
     */
    static std::string roundToString(uint64_t value){
        std::string result = std::to_string(value);

        // add commas for thousands
        for(int i = (int)result.length()-3; i > 0; i -= 3){
            result.insert(i, ",");
        }

        return result;
    }

    /**
     * Rounds a given value and returns it as string.
     * 
     * @param value Value to round
     * @return std::string Value as string.
     */
    static std::string roundToString(int64_t value){
        std::string result = std::to_string(value);
        if(value < 0){
            result = result.substr(1); // strip minus sign
        }

        // add commas for thousands
        for(int i = (int)result.length()-3; i > 0; i -= 3){
            result.insert(i, ",");
        }

        return (value < 0 ? "-" : "")+result;
    }


    /**
     * Rounds a given value with a given precision and returns it as string.
     * 
     * @param value Value to round
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Value as string
     */
    static std::string roundToString(double value, size_t digitsAfterComma = 1){
        size_t factor = 1 << digitsAfterComma;
        value = std::round(value * (double)factor) / (double)factor;
        uint64_t intValue = (uint64_t)value;
        double remainder = value - (double)intValue;
        
        std::string result = std::to_string(intValue);

        // add commas for thousands
        for(int i = (int)result.length()-3; i > 0; i -= 3){
            result.insert(i, ",");
        }

        // add remainder
        if(digitsAfterComma > 0){
            result += ".";
            for(size_t i = 0; i < digitsAfterComma; i++){
                remainder *= 10;
                result += std::to_string((uint64_t)remainder);
                remainder -= (uint64_t)remainder;
            }
        }

        return result;
    }



    /**
     * Converts a given byte size into a human readable string e.g. 3GB, 5.5TB, ...
     * 
     * @param byteSize Byte count to convert
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Human readable string
     */
    static std::string byteSizeToString(uint64_t byteSize, size_t digitsAfterComma = 1, ByteSizeMetric metric = ByteSizeMetric::KILOBYTE){
        uint64_t prevLimit = 1, limit = (uint64_t)metric;
        std::string hasI = metric == ByteSizeMetric::KILOBYTE ? "" : "i";
        if(byteSize < limit) return std::to_string(byteSize)+"B";
        prevLimit = limit; limit *= (uint64_t)metric;
        if(byteSize < limit) return roundToString((double)byteSize / (double)prevLimit, digitsAfterComma)+"K"+hasI+"B";
        prevLimit = limit; limit *= (uint64_t)metric;
        if(byteSize < limit) return roundToString((double)byteSize / (double)prevLimit, digitsAfterComma)+"M"+hasI+"B";
        prevLimit = limit; limit *= (uint64_t)metric;
        if(byteSize < limit) return roundToString((double)byteSize / (double)prevLimit, digitsAfterComma)+"G"+hasI+"B";
        prevLimit = limit; limit *= (uint64_t)metric;
        if(byteSize < limit) return roundToString((double)byteSize / (double)prevLimit, digitsAfterComma)+"T"+hasI+"B";
        prevLimit = limit; limit *= (uint64_t)metric;
        return roundToString((double)byteSize / (double)prevLimit, digitsAfterComma)+"P"+hasI+"B";
    }

    /**
     * Converts a given bandwidth (bytes per second) into a human readable string e.g. 3GB/s, 5.5TB/s, ...
     * 
     * @param byteSize Byte count per second to convert
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Human readable string
     */
    static std::string bytesPerSecToString(uint64_t byteCount, size_t digitsAfterComma = 1){
        return byteSizeToString(byteCount, digitsAfterComma)+"/s";
    }

    /**
     * Converts a given bandwidth (bytes per millisecond) into a human readable string e.g. 3GB/s, 5.5TB/s, ...
     * 
     * @param byteSize Byte count per millisecond to convert
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Human readable string
     */
    static std::string bytesPerMilliSecToString(uint64_t byteCount, size_t digitsAfterComma = 1){
        // TODO improve to allow larger byteCount before overflow
        return bytesPerSecToString(byteCount*1000, digitsAfterComma); // naive (faster overflow)
    }

    /**
     * Converts a given bandwidth (bytes per microsecond) into a human readable string e.g. 3GB/s, 5.5TB/s, ...
     * 
     * @param byteSize Byte count per microsecond to convert
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Human readable string
     */
    static std::string bytesPerMicroSecToString(uint64_t byteCount, size_t digitsAfterComma = 1){
        // TODO improve to allow larger byteCount before overflow
        return bytesPerSecToString(byteCount*1000*1000, digitsAfterComma); // naive (faster overflow)
    }




    /**
     * Converts a given duration in milliseconds into a human readable string e.g. 3.5sec
     * 
     * @param microseconds Duration in milliseconds
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Human readable string
     */
    static std::string millisecondsToString(int64_t microseconds, size_t digitsAfterComma = 1){
        const uint64_t absMicroseconds = (uint64_t)(microseconds < 0 ? -microseconds : microseconds);
        uint64_t prevLimit = 1, limit = 1000;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"ms";
        prevLimit = limit; limit *= 1000;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"s";
        prevLimit = limit; limit *= 60;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"m";
        prevLimit = limit; limit *= 60;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"h";
        prevLimit = limit; limit *= 24;
        return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"d";
    }

    /**
     * Converts a given duration in microseconds into a human readable string e.g. 3.5sec
     * 
     * @param microseconds Duration in microseconds
     * @param digitsAfterComma Amount of digits after comma (precision)
     * @return std::string Human readable string
     */
    static std::string microsecondsToString(int64_t microseconds, size_t digitsAfterComma = 1){
        const uint64_t absMicroseconds = (uint64_t)(microseconds < 0 ? -microseconds : microseconds);
        uint64_t prevLimit = 1, limit = 1000;
        if(absMicroseconds < limit) return std::to_string(microseconds)+"us";
        prevLimit = limit; limit *= 1000;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"ms";
        prevLimit = limit; limit *= 1000;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"s";
        prevLimit = limit; limit *= 60;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"m";
        prevLimit = limit; limit *= 60;
        if(absMicroseconds < limit) return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"h";
        prevLimit = limit; limit *= 24;
        return roundToString((double)microseconds / (double)prevLimit, digitsAfterComma)+"d";
    }

};


}
#endif // SPI_METRICS_UTILS_HPP