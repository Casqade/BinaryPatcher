#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <type_traits>


class NumberValue
{
  alignas(uint64_t) uint8_t mStorage[sizeof(uint64_t)] {};

  uint8_t mByteCount {};

  enum class NumberType
  {
    Integral,
    Float,
    Double,

  } mType {};

  bool mSigned {};


public:

  NumberValue() = default;

  template <typename T,
    std::enable_if_t <std::is_integral_v <T>, int> = 0>
  NumberValue( T value )
    : mByteCount{ sizeof(T) }
    , mSigned{ std::is_signed_v <T> }
  {
    std::memcpy(mStorage, &value, sizeof(T));
  }

  template <typename T,
    std::enable_if_t <std::is_same_v <T, float>, int> = 0>
  NumberValue( T value )
    : mByteCount{ sizeof(T) }
    , mType{NumberType::Float}
  {
    std::memcpy(mStorage, &value, sizeof(T));
  }

  template <typename T,
    std::enable_if_t <std::is_same_v <T, double>, int> = 0>
  NumberValue( T value )
    : mByteCount{ sizeof(T) }
    , mType{NumberType::Double}
  {
    std::memcpy(mStorage, &value, sizeof(T));
  }

  static NumberValue fromInt64(
    int64_t value, uint8_t byteWidth, bool isSigned );

  static NumberValue fromUint64(
    uint64_t value, uint8_t byteWidth );

  static NumberValue fromFloat( float value );
  static NumberValue fromDouble( double value );

  bool operator < ( const NumberValue& ) const;

  bool isSigned() const;
  uint8_t byteWidth() const;

  std::vector <uint8_t> toBytes() const;

  int64_t asInt64() const;
  uint64_t asUint64() const;
  float asFloat() const;
  double asDouble() const;
  std::string asString() const;

  void setFromInt64( int64_t );
  void setFromUint64( uint64_t );
  void setFromFloat( float );
  void setFromDouble( double );
  void setFromString( const std::string& );
  bool setFromString( const std::string&, const NumberValue& min, const NumberValue& max );
};


inline
NumberValue
NumberValue::fromInt64(
  int64_t value,
  uint8_t byteCount,
  bool isSigned )
{
  NumberValue result;

  result.mByteCount = byteCount;
  result.mSigned = isSigned;

  std::memcpy(result.mStorage, &value, byteCount);

  return result;
}

inline
NumberValue
NumberValue::fromUint64(
  uint64_t value,
  uint8_t byteCount )
{
  NumberValue result;

  result.mByteCount = byteCount;

  std::memcpy(result.mStorage, &value, byteCount);

  return result;
}

inline
NumberValue
NumberValue::fromFloat(
  float value )
{
  NumberValue result;

  result.mByteCount = sizeof(value);
  result.mSigned = false;
  result.mType = NumberType::Float;

  std::memcpy(result.mStorage, &value, sizeof(value));

  return result;
}

inline
NumberValue
NumberValue::fromDouble(
  double value )
{
  NumberValue result;

  result.mByteCount = sizeof(value);
  result.mSigned = false;
  result.mType = NumberType::Double;

  std::memcpy(result.mStorage, &value, sizeof(value));

  return result;
}

inline
bool
NumberValue::operator < (
  const NumberValue& other ) const
{
  switch (mType)
  {
    case NumberType::Integral:
    {
      if ( mSigned == true )
        return asInt64() < other.asInt64();
      else
        return asUint64() < other.asUint64();
    }

    case NumberType::Float:
      return asFloat() < other.asFloat();

    case NumberType::Double:
      return asDouble() < other.asDouble();

    default:
      return false;
  }
}

inline
bool
NumberValue::isSigned() const
{
  return mSigned;
}

inline
uint8_t
NumberValue::byteWidth() const
{
  return mByteCount;
}

inline
std::vector <uint8_t>
NumberValue::toBytes() const
{
  return {mStorage, mStorage + mByteCount};
}

inline
int64_t
NumberValue::asInt64() const
{
  int64_t value {};

//  if ( mByteCount < sizeof(int64_t) && (mStorage[mByteCount - 1] & 0x80) )
//    std::memset(&value, 0xFF, sizeof(value));

  if ( (mStorage[mByteCount - 1] & 0x80) != 0 )
    value = ~value;

  std::memcpy(&value, mStorage, mByteCount);
  return value;
}

inline
uint64_t
NumberValue::asUint64() const
{
  uint64_t value {};
  std::memcpy(&value, mStorage, mByteCount);
  return value;
}

inline
float
NumberValue::asFloat() const
{
  float value {};
  std::memcpy(&value, mStorage, mByteCount);
  return value;
}

inline
double
NumberValue::asDouble() const
{
  double value {};
  std::memcpy(&value, mStorage, mByteCount);
  return value;
}

inline
std::string
NumberValue::asString() const
{
  switch (mType)
  {
    case NumberType::Integral:
    {
      if ( mSigned )
        return std::to_string( asInt64() );
      else
        return std::to_string( asUint64() );
    }

    case NumberType::Float:
    {
      std::ostringstream stream {};
      stream << std::fixed << asFloat();
      return stream.str();
    }

    case NumberType::Double:
    {
      std::ostringstream stream {};
      stream << std::fixed << asDouble();
      return stream.str();
    }

    default:
      return {};
  }
}

inline
void
NumberValue::setFromInt64(
  int64_t value )
{
  std::memcpy(mStorage, &value, mByteCount);
}

inline
void
NumberValue::setFromUint64(
  uint64_t value )
{
  std::memcpy(mStorage, &value, mByteCount);
}

inline
void
NumberValue::setFromFloat(
  float value )
{
  std::memcpy(mStorage, &value, mByteCount);
}

inline
void
NumberValue::setFromDouble(
  double value )
{
  std::memcpy(mStorage, &value, mByteCount);
}

inline
void
NumberValue::setFromString(
  const std::string& str )
{
  switch (mType)
  {
    case NumberType::Integral:
    {
      if ( mSigned == true )
      {
        int64_t value = std::stoll(str, nullptr, 0);
        std::memcpy(mStorage, &value, mByteCount);
      }
      else
      {
        if ( str.size() > 0 && str[0] == '-' )
          throw std::runtime_error(
            "Can't initialize unsigned integer with negative value" );

        uint64_t value = std::stoull(str, nullptr, 0);
        std::memcpy(mStorage, &value, mByteCount);
      }

      break;
    }

    case NumberType::Float:
    {
      float value = std::stof(str);
      std::memcpy(mStorage, &value, mByteCount);

      break;
    }

    case NumberType::Double:
    {
      double value = std::stod(str);
      std::memcpy(mStorage, &value, mByteCount);

      break;
    }
  }
}

inline
bool
NumberValue::setFromString(
  const std::string& str,
  const NumberValue& min,
  const NumberValue& max )
{
  switch (mType)
  {
    case NumberType::Integral:
    {
      if ( mSigned == true )
      {
        int64_t value = std::stoll(str, nullptr, 0);

        if ( value >= min.asInt64() && value <= max.asInt64() )
        {
          std::memcpy(mStorage, &value, mByteCount);
          return true;
        }
      }
      else
      {
        if ( str.size() > 0 && str[0] == '-' )
          throw std::runtime_error(
            "Can't initialize unsigned integer with negative value" );

        uint64_t value = std::stoull(str, nullptr, 0);

        if ( value >= min.asUint64() && value <= max.asUint64() )
        {
          std::memcpy(mStorage, &value, mByteCount);
          return true;
        }
      }

      break;
    }

    case NumberType::Float:
    {
      float value = std::stof(str);

      if ( value >= min.asFloat() && value <= max.asFloat() )
      {
        std::memcpy(mStorage, &value, mByteCount);
        return true;
      }

      break;
    }

    case NumberType::Double:
    {
      double value = std::stod(str);

      if ( value >= min.asDouble() && value <= max.asDouble() )
      {
        std::memcpy(mStorage, &value, mByteCount);
        return true;
      }

      break;
    }
  }

  return false;
}
