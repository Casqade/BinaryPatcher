#pragma once

#include "../NumberValue.hpp"

#include <array>
#include <istream>
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
    Restore,
    Modify,
  };

  using OperationModeOrder = std::array <OperationMode, 3>;


  virtual ~Patch() = default;

  virtual void prompt() {}
  virtual void apply( std::iostream&, OperationMode ) const {}


  static void Apply(
    std::ostream&,
    std::ios::off_type offset,
    const ByteSequence& bytes );

  static void Apply(
    std::ostream&,
    OperationMode mode,
    std::ios::off_type offset,
    const ByteSequence& originalBytes,
    const ByteSequence& modifiedBytes );


  static void ReadBytes(
    std::istream&,
    std::ios::off_type offset,
    char* dst,
    size_t count );


  static ByteSequence ToBytes(
    const std::string& text );

  static ByteSequence ToBytes(
    uint64_t value, size_t byteCount );


  static OperationMode PromptOperationMode(
    const std::string& description,
    const OperationModeOrder& modeOrder );

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
  std::ostream& stream,
  std::ios::off_type offset,
  const ByteSequence& bytes )
{
  stream.seekp( offset, std::ios::beg );

  stream.write(
    reinterpret_cast <const char*> (bytes.data()),
    bytes.size() );
}

inline
void
Patch::Apply(
  std::ostream& stream,
  OperationMode mode,
  std::ios::off_type offset,
  const ByteSequence& originalBytes,
  const ByteSequence& modifiedBytes )
{
  if ( mode == OperationMode::Modify )
    Apply(stream, offset, modifiedBytes);

  else if ( mode == OperationMode::Restore )
    Apply(stream, offset, originalBytes);
}

inline
void
Patch::ReadBytes(
  std::istream& stream,
  std::ios::off_type offset,
  char* dst,
  size_t count )
{
  stream.seekg( offset, std::ios::beg );
  stream.read( dst, count );
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
  const std::string& description,
  const OperationModeOrder& modeOrder )
{
  OperationMode mode {};

  struct ModeStrings
  {
    std::string name;
    std::string shortName;
    std::string description;
  };

  const static ModeStrings modeStrings[]
  {
    { "skip", "s", " (default, don't write to executable)" },
    { "no", "n", " (restore bytes from original binary)" },
    { "yes", "y", "" },
  };

  auto modeToStrings =
  [] ( OperationMode mode ) -> const ModeStrings&
  {
    auto modeIndex = static_cast <size_t> (mode);
    return modeStrings[modeIndex];
  };

  while ( true )
  {
    std::cout << description << "\n\n"
      "  Enable? (1/2/3):\n"
      "    1. " << modeToStrings(modeOrder[0]).name << modeToStrings(modeOrder[0]).description << "\n"
      "    2. " << modeToStrings(modeOrder[1]).name << modeToStrings(modeOrder[1]).description << "\n"
      "    3. " << modeToStrings(modeOrder[2]).name << modeToStrings(modeOrder[2]).description << "\n";

    std::string userInput {};
    std::getline(std::cin, userInput);

    if ( userInput.empty() == true )
    {
      mode = modeOrder[0];
      break;
    }

    for ( size_t i {}; i < modeOrder.size(); ++i )
    {
      if (  userInput == std::to_string(i + 1) ||
            userInput == modeToStrings(modeOrder[i]).shortName ||
            userInput == modeToStrings(modeOrder[i]).name )
      {
        std::cout << "\n";
        return modeOrder[i];
      }
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
