#pragma once

#include "../NumberValue.hpp"

#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <functional>

using ByteSequence = std::vector <uint8_t>;


class Patch
{
public:

  enum class OperationMode
  {
    Skip,
    Modify,
    Restore,
  };

  virtual ~Patch() = default;

  virtual void prompt() {}
  virtual void apply( std::fstream&, OperationMode ) const {}


  static void Apply(
    std::fstream& file,
    std::ios::off_type offset,
    const ByteSequence& bytes );

  static void Apply(
    std::fstream& file,
    OperationMode mode,
    std::ios::off_type offset,
    const ByteSequence& originalBytes,
    const ByteSequence& modifiedBytes );


  static void ReadBytes(
    std::fstream& file,
    std::ios::off_type offset,
    char* dst,
    size_t count );


  static ByteSequence ToBytes(
    const std::string& text );

  static ByteSequence ToBytes(
    uint64_t value, size_t byteCount );


  static OperationMode PromptOperationMode(
    const std::string& description );

  static void PromptNumber(
    NumberValue& number,
    const NumberValue& rangeMin,
    const NumberValue& rangeMax,
    const std::string& numberName );


  using TextValidationCallback =
    std::function <bool( const std::string& text )>;

  void PromptText(
    std::string& text,
    TextValidationCallback validateText,
    const std::string& textName );
};


inline
void
Patch::Apply(
  std::fstream& file,
  std::ios::off_type offset,
  const ByteSequence& bytes )
{
  file.seekp( offset, std::ios::beg );

  file.write(
    reinterpret_cast <const char*> (bytes.data()),
    bytes.size() );
}

inline
void
Patch::Apply(
  std::fstream& file,
  OperationMode mode,
  std::ios::off_type offset,
  const ByteSequence& originalBytes,
  const ByteSequence& modifiedBytes )
{
  if ( mode == OperationMode::Modify )
    Apply(file, offset, modifiedBytes);

  else if ( mode == OperationMode::Restore )
    Apply(file, offset, originalBytes);
}

inline
void
Patch::ReadBytes(
  std::fstream& file,
  std::ios::off_type offset,
  char* dst,
  size_t count )
{
  file.seekg( offset, std::ios::beg );
  file.read( dst, count );
}

inline
ByteSequence
Patch::ToBytes(
  uint64_t value,
  size_t byteCount )
{
  ByteSequence bytes(byteCount, 0);

  std::memcpy(
    bytes.data(),
    &value, byteCount );

  return bytes;
}

inline
ByteSequence
Patch::ToBytes(
  const std::string& text )
{
  ByteSequence bytes(text.size());

  std::memcpy(
    bytes.data(),
    text.data(),
    text.size() );

  return bytes;
}

inline
Patch::OperationMode
Patch::PromptOperationMode(
  const std::string& description )
{
  OperationMode mode {};

  while ( true )
  {
    std::cout << description << "\n\n"
      "  Enable? (1/2/3):\n"
      "    1. skip (default, don't write to executable)\n"
      "    2. no (restore bytes from original binary)\n"
      "    3. yes\n";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if (  userInput.empty() == true ||
          userInput == "1" ||
          userInput == "s" ||
          userInput == "skip" )
    {
      mode = OperationMode::Skip;
      break;
    }

    if (  userInput == "2" ||
          userInput == "n" ||
          userInput == "no" )
    {
      mode = OperationMode::Restore;
      break;
    }

    if (  userInput == "3" ||
          userInput == "y" ||
          userInput == "yes" )
    {
      mode = OperationMode::Modify;
      break;
    }

    std::cout << "ERROR: Unrecognized input: '" << userInput << "'\n\n";
  }

  std::cout << "\n";

  return mode;
}

inline
void
Patch::PromptNumber(
  NumberValue& number,
  const NumberValue& rangeMin,
  const NumberValue& rangeMax,
  const std::string& numberName )
{
  while ( true )
  {
    std::cout
      << "Set " << numberName
      << " or leave empty to use default ("
      << number.asString()
      << "), allowed range ["
      << rangeMin.asString() << ", "
      << rangeMax.asString() << "]: ";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if ( userInput.empty() == true )
      break;

    try
    {
      if ( number.setFromString(userInput, rangeMin, rangeMax) == true )
        break;
    }
    catch ( ... ) {}

    std::cout << "ERROR: Unsupported " << numberName << " '" << userInput << "'\n\n";
  }

  std::cout
    << numberName << " set to "
    << number.asString() << "\n\n";
}

inline
void
Patch::PromptText(
  std::string& text,
  TextValidationCallback validateText,
  const std::string& textName )
{
  while ( true )
  {
    std::cout
      << "Set " << textName
      << " or leave empty to use default '"
      << text << "': ";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if ( userInput.empty() == true )
      break;


    if ( validateText(userInput) == true )
    {
      text = userInput;
      break;
    }

    std::cout << "\n";
  }

  std::cout
    << textName << " set to "
    "'" << text << "'\n\n";
}
