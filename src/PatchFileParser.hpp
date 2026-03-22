#pragma once

#include "Patches/PatchFile.hpp"
#include "Patches/BinaryPatch.hpp"
#include "Patches/FlagsPatch.hpp"
#include "Patches/MessagePatch.hpp"
#include "Patches/NumberPatch.hpp"
#include "Patches/StringPatch.hpp"

#include <set>
#include <limits>
#include <memory>
#include <sstream>
#include <unordered_map>


class PatchFileParser
{
  Patch::OperationModeOrder mOperationOrder
  {
    Patch::OperationMode::Skip,
    Patch::OperationMode::Restore,
    Patch::OperationMode::Modify,
  };

  std::vector <std::string> mLines;
  size_t mCurrentLine {};
  size_t mExecutableSize {};


public:

  PatchFile parse( const std::string& filename );

  const Patch::OperationModeOrder& operationOrder() const
  {
    return mOperationOrder;
  }


private:

  static std::string TrimWhitespace( const std::string& str );

  uint64_t ParseHexValue( std::string );

  std::vector <uint64_t> ParseOffsetList(
    const std::string& line );

  ByteSequence ParseHexByteSequence(
    const std::string& line );

  void skipEmptyAndComments();
  std::string parseMultilineString();
  Patch::OperationModeOrder parseOperationOrder( std::istringstream& ) const;

  std::unique_ptr <Patch> parseBinaryEntry();
  std::unique_ptr <Patch> parseFlagsEntry(const std::string& header);
  std::unique_ptr <Patch> parseMessageEntry();
  std::unique_ptr <Patch> parseStringEntry( const std::string& header );
  std::unique_ptr <Patch> parseNumberEntry( const std::string& header );
  std::unique_ptr <Patch> parseEntry();
  PatchGroup parseGroup();

  std::string currentLine() const
  {
    return mCurrentLine < mLines.size()
      ? TrimWhitespace(mLines[mCurrentLine])
      : "";
  }

  void advance()
  {
    ++mCurrentLine;
  }

  bool atEnd() const
  {
    return mCurrentLine >= mLines.size();
  }


  bool isComment( const std::string& line ) const
  {
    return
      line.size() >= 2 &&
      line[0] == '/' &&
      line[1] == '/';
  }
};

inline
std::string
PatchFileParser::TrimWhitespace(
  const std::string& str )
{
  const auto start = str.find_first_not_of(" \t\r\n");

  if ( start == std::string::npos )
    return "";

  const auto end = str.find_last_not_of(" \t\r\n");

  return str.substr(start, end - start + 1);
}

inline
uint64_t
PatchFileParser::ParseHexValue(
  std::string str )
{
  str = TrimWhitespace(str);

  try
  {
    auto result = std::stoull(str, nullptr, 16);
    return result;
  }
  catch ( const std::exception& e )
  {
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) + ": "
      "'" + str + "' is not a valid hex value");
  }
}

inline
std::vector <uint64_t>
PatchFileParser::ParseOffsetList(
  const std::string& line )
{
  std::vector <uint64_t> offsets;
  std::istringstream stream(line);
  std::string token;

  while ( std::getline(stream, token, ',') )
  {
    token = TrimWhitespace(token);

    auto commentPos = token.find("//");

    token = token.substr(0, commentPos);

    if ( token.empty() == false )
    {
      uint64_t offset = ParseHexValue(token);
      offsets.push_back(offset);

      if ( offsets.back() >= mExecutableSize )
        throw std::runtime_error(
          "Line " + std::to_string(mCurrentLine + 1) + ": "
          "offset " + std::to_string(offset) +
          " exceeds executable size (" + std::to_string(mExecutableSize) + ")" );
    }

    if ( commentPos != std::string::npos )
      break;
  }

  return offsets;
}

inline
ByteSequence
PatchFileParser::ParseHexByteSequence(
  const std::string& line )
{
  ByteSequence bytes;
  std::istringstream stream(line);
  std::string token;

  while ( stream >> token )
  {
    if ( token.find("//") != std::string::npos )
      return bytes;

    uint64_t val = ParseHexValue(token);

    if ( val > 0xFF )
      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine + 1) +
        ": Invalid byte value '" + token + "'");

    bytes.push_back(static_cast <uint8_t> (val));
  }

  return bytes;
}

inline
void
PatchFileParser::skipEmptyAndComments()
{
  while ( atEnd() == false )
  {
    const auto line = currentLine();

    if ( line.empty() || isComment(line) )
      advance();
    else
      break;
  }
}

inline
std::string
PatchFileParser::parseMultilineString()
{
  std::string result;
  bool first = true;

  while ( atEnd() == false )
  {
    if ( mCurrentLine >= mLines.size() )
      break;

    auto line = mLines[mCurrentLine];

    const auto start = line.find_first_not_of(" \t\r\n");

    if ( start == std::string::npos )
      break;

    line = line.substr(start);


    if ( line.empty() || line[0] != '#' )
      break;

    if ( first == false )
      result += "\n";

    first = false;

    if ( line == "#" )
      ; // empty line in multiline string

    else if ( line.size() >= 2 && line[0] == '#' && line[1] == ' ' )
      result += line.substr(2);

    else
      break;

    advance();
  }

  return result;
}

inline
std::unique_ptr <Patch>
PatchFileParser::parseBinaryEntry()
{
  advance(); // skip "binary"

  skipEmptyAndComments();
  auto offsets = ParseOffsetList(currentLine());
  advance();

  skipEmptyAndComments();
  auto originalBytes = ParseHexByteSequence(currentLine());
  advance();

  for ( const auto& offset : offsets )
    if ( offset + originalBytes.size() > mExecutableSize )
      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine) +
        ": " + std::to_string(originalBytes.size()) +
        " bytes at offset " + std::to_string(offset) +
        " exceed executable size (" + std::to_string(mExecutableSize) + ")" );

  skipEmptyAndComments();
  auto modifiedBytes = ParseHexByteSequence(currentLine());
  advance();

  if ( modifiedBytes.size() != originalBytes.size() )
    throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine) +
        ": Modified byte count must be equal to original bytes count (expected " + std::to_string(originalBytes.size()) +
        ", got " + std::to_string(modifiedBytes.size()) + ")" );


  return std::make_unique <BinaryPatch> (
    std::move(offsets),
    std::move(originalBytes),
    std::move(modifiedBytes) );
}

inline
std::unique_ptr <Patch>
PatchFileParser::parseFlagsEntry(const std::string& header)
{
  std::istringstream stream(header);
  std::string keyword;
  stream >> keyword; // "flags"

  bool unset = false;

  std::string extra;
  if ( stream >> extra && extra == "unset" )
    unset = true;

  advance(); // skip header line

  skipEmptyAndComments();
  auto offsets = ParseOffsetList(currentLine());
  advance();

  skipEmptyAndComments();
  std::string hexPart = currentLine();

  if ( hexPart.size() >= 2 && hexPart[0] == '0' && (hexPart[1] == 'x') )
    hexPart = hexPart.substr(2);

  size_t digits = hexPart.size();
  size_t byteWidth = (digits + 1) / 2;

  if ( byteWidth > 4 )       byteWidth = 8;
  else if ( byteWidth > 2 )  byteWidth = 4;
  else if ( byteWidth > 1 )  byteWidth = 2;

  uint64_t mask = std::stoull(hexPart, nullptr, 16);
  advance();

  for ( const auto& offset : offsets )
    if ( offset + byteWidth > mExecutableSize )
      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine) +
        ": " + std::to_string(byteWidth) +
        "-byte long flags at offset " + std::to_string(offset) +
        " exceed executable size (" + std::to_string(mExecutableSize) + ")" );

  return std::make_unique <FlagsPatch> (
    std::move(offsets), mask, byteWidth, unset );
}

inline
std::unique_ptr <Patch>
PatchFileParser::parseMessageEntry()
{
  advance(); // skip "message"

  skipEmptyAndComments();
  auto text = parseMultilineString();

  return std::make_unique <MessagePatch> (
    std::move(text) );
}

inline
std::unique_ptr <Patch>
PatchFileParser::parseStringEntry(
  const std::string& header )
{
  std::istringstream stream(header);
  std::string keyword;
  stream >> keyword; // "string"

  size_t maxLength {};
  bool urlValidation = false;

  stream >> maxLength;

  std::string extra;
  if ( stream >> extra && extra == "url" )
    urlValidation = true;

  advance(); // skip header line

  skipEmptyAndComments();
  auto name = parseMultilineString();

  skipEmptyAndComments();
  auto offsets = ParseOffsetList(currentLine());
  advance();

  for ( const auto& offset : offsets )
    if ( offset + maxLength > mExecutableSize )
      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine) +
        ": Max string size " + std::to_string(maxLength) +
        " at offset " + std::to_string(offset) +
        " will exceed executable size (" + std::to_string(mExecutableSize) + ")" );

  skipEmptyAndComments();
  auto originalString = parseMultilineString();

  if ( originalString.size() > maxLength )
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) +
      ": Original string '" + originalString + "'"
      " exceeds max length of " +
      std::to_string(maxLength) + " bytes" );

  skipEmptyAndComments();
  auto replacedString = parseMultilineString();

  if ( replacedString.size() > maxLength )
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) +
      ": Replaced string '" + replacedString + "'"
      " exceeds max length of " +
      std::to_string(maxLength) + " bytes" );

  return std::make_unique <StringPatch> (
    std::move(offsets),
    std::move(name),
    maxLength,
    urlValidation,
    std::move(originalString),
    std::move(replacedString) );
}


struct NumberTypeInfo
{
  NumberValue minLimit {};
  NumberValue maxLimit {};

  NumberValue fromInt64( int64_t value ) const
  {
    return NumberValue::fromInt64(
      value,
      minLimit.byteWidth(),
      minLimit.isSigned() );
  }

  NumberValue fromUint64( uint64_t value ) const
  {
    return NumberValue::fromUint64(
      value, minLimit.byteWidth() );
  }

  NumberValue fromFloat( float value ) const
  {
    return NumberValue::fromFloat(value);
  }

  NumberValue fromDouble( double value ) const
  {
    return NumberValue::fromDouble(value);
  }
};

template <typename T>
static
NumberTypeInfo
makeNumberType()
{
  return {
    NumberValue {std::numeric_limits <T>::min()},
    NumberValue {std::numeric_limits <T>::max()},
  };
}

static const std::unordered_map <std::string, NumberTypeInfo> numberTypeMap
{
  {"uint8",  makeNumberType <uint8_t>()},
  {"int8",   makeNumberType <int8_t>()},
  {"uint16", makeNumberType <uint16_t>()},
  {"int16",  makeNumberType <int16_t>()},
  {"uint32", makeNumberType <uint32_t>()},
  {"int32",  makeNumberType <int32_t>()},
  {"uint64", makeNumberType <uint64_t>()},
  {"int64",  makeNumberType <int64_t>()},
  {"float",  makeNumberType <float>()},
  {"double",  makeNumberType <double>()},
};


inline
std::unique_ptr <Patch>
PatchFileParser::parseNumberEntry(
  const std::string& header )
{
  std::istringstream stream(header);
  std::string keyword, typeName;
  stream >> keyword >> typeName;

  auto it = numberTypeMap.find(typeName);

  if ( it == numberTypeMap.end() )
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) +
      ": Unknown number type '" + typeName + "'" );

  const auto& typeInfo = it->second;

  NumberValue rangeMin = typeInfo.minLimit;
  NumberValue rangeMax = typeInfo.maxLimit;

  if ( stream >> keyword )
  {
    std::string value;

    if ( keyword == "min" && stream >> value )
    {
      rangeMin.setFromString(value);
      stream >> keyword;
    }

    if ( keyword == "max" && stream >> value )
      rangeMax.setFromString(value);

    if ( rangeMax < rangeMin )
      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine + 1) +
        ": Invalid number range, max (" +
        rangeMax.asString() + ") < min (" +
        rangeMin.asString() + ")" );
  }

  advance(); // skip header line

  skipEmptyAndComments();
  auto name = parseMultilineString();

  skipEmptyAndComments();
  auto offsets = ParseOffsetList(currentLine());
  advance();

  for ( const auto& offset : offsets )
    if ( offset + rangeMin.byteWidth() > mExecutableSize )
      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine) +
        ": " + std::to_string(rangeMin.byteWidth()) +
        "-byte number at offset " + std::to_string(offset) +
        " exceeds executable size (" + std::to_string(mExecutableSize) + ")" );

  skipEmptyAndComments();
  auto originalNum = rangeMin;
  originalNum.setFromString(currentLine());
  advance();

  skipEmptyAndComments();
  auto defaultNum = rangeMin;
  defaultNum.setFromString(currentLine());
  advance();

  return std::make_unique <NumberPatch> (
    std::move(offsets),
    std::move(name),
    rangeMin, rangeMax,
    originalNum, defaultNum );
}

inline
std::unique_ptr <Patch>
PatchFileParser::parseEntry()
{
  skipEmptyAndComments();
  const auto line = currentLine();

  bool ignoreEntry {};
  std::unique_ptr <Patch> entry {};

  std::istringstream ss(line);
  std::string keyword;
  ss >> keyword;

  if ( keyword[0] == '-' )
  {
    ignoreEntry = true;
    keyword.erase(0, 1);
  }

  if ( keyword == "binary" )
    entry = parseBinaryEntry();

  else if ( keyword == "flags" )
    entry = parseFlagsEntry(line);

  else if ( keyword == "message" )
    entry = parseMessageEntry();

  else if ( keyword == "string" )
    entry = parseStringEntry(line);

  else if ( keyword == "number" )
    entry = parseNumberEntry(line);

  if ( ignoreEntry == true )
    return {};

  if ( entry == nullptr )
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) + ": "
      "Unknown patch entry type '" + line + "'" );

  return entry;
}

inline
PatchGroup
PatchFileParser::parseGroup()
{
  PatchGroup group {};

  skipEmptyAndComments();
  group.name = currentLine();
  advance();

  skipEmptyAndComments();

  if ( atEnd() == false && currentLine()[0] == '#' )
    group.description = parseMultilineString();

  skipEmptyAndComments();

  if ( currentLine() != "{" )
    throw std::runtime_error(
      "Expected '{' at line " + std::to_string(mCurrentLine + 1)
      + ", got: '" + currentLine() + "'" );

  advance();

  while ( atEnd() == false )
  {
    skipEmptyAndComments();

    if ( atEnd() || currentLine() == "}" )
      break;

    auto entry = parseEntry();

    if ( entry != nullptr )
      group.entries.push_back(std::move(entry));
  }

  if ( atEnd() || currentLine() != "}" )
    throw std::runtime_error(
      "Expected '}' to close group '" + group.name + "'" );

  advance();

  if ( group.name[0] == '-' )
    group.entries.clear();

  return group;
}

inline
Patch::OperationModeOrder
PatchFileParser::parseOperationOrder(
  std::istringstream& stream ) const
{
  auto operationOrder = mOperationOrder;

  std::string operationMode;

  using OperationMode = Patch::OperationMode;
  std::set <OperationMode> operationModes
  {
    OperationMode::Skip,
    OperationMode::Modify,
    OperationMode::Restore,
  };

  for ( size_t i {}; i < 3; ++i )
  {
    if ( stream >> operationMode )
    {
      if ( operationMode == "yes" )
      {
        if ( operationModes.count(OperationMode::Modify) == 0 )
          throw std::runtime_error(
            "Line " + std::to_string(mCurrentLine + 1) + ": "
            "Duplicate operation mode 'yes'" );

        operationOrder[i] = OperationMode::Modify;
        operationModes.erase(OperationMode::Modify);
        continue;
      }

      else if ( operationMode == "no" )
      {
        if ( operationModes.count(OperationMode::Restore) == 0 )
          throw std::runtime_error(
            "Line " + std::to_string(mCurrentLine + 1) + ": "
            "Duplicate operation mode 'no'" );

        operationOrder[i] = OperationMode::Restore;
        operationModes.erase(OperationMode::Restore);
        continue;
      }

      else if ( operationMode == "skip" )
      {
        if ( operationModes.count(OperationMode::Skip) == 0 )
          throw std::runtime_error(
            "Line " + std::to_string(mCurrentLine + 1) + ": "
            "Duplicate operation mode 'skip'" );

        operationOrder[i] = OperationMode::Skip;
        operationModes.erase(OperationMode::Skip);
        continue;
      }

      throw std::runtime_error(
        "Line " + std::to_string(mCurrentLine + 1) + ": "
        "Invalid operation mode '" + operationMode + "'" );
    }

    if ( i == 0 )
      break;

    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) + ": "
      "All 3 operation modes must be defined (yes/no/skip)" );
  }

  return operationOrder;
}

inline
PatchFile
PatchFileParser::parse(
  const std::string& filename )
{
  PatchFile result {};

  std::ifstream file(filename);

  if ( file.is_open() == false )
    throw std::runtime_error("Failed to open file");

  std::string line;

  while ( std::getline(file, line) )
    mLines.push_back(line);

  mCurrentLine = 0;

  skipEmptyAndComments();
  std::istringstream stream(currentLine());
  stream >> result.executableName >> result.executableSize;

  if ( result.executableName.empty() == true )
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) + ": "
      "Executable name can't be empty" );

  if ( result.executableSize == 0 )
    throw std::runtime_error(
      "Line " + std::to_string(mCurrentLine + 1) + ": "
      "Executable size can't be zero" );

  mExecutableSize = result.executableSize;
  mOperationOrder = parseOperationOrder(stream);

  advance();

  while ( atEnd() == false )
  {
    skipEmptyAndComments();

    if ( atEnd() )
      break;

    result.groups.push_back(parseGroup());
  }

  return result;
}
