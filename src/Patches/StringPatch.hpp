#include "Patch.hpp"

#include <cstdint>
#include <vector>
#include <ios>


class StringPatch : public Patch
{
  std::vector <uint64_t> mOffsets;

  std::string mStringName;

  size_t mMaxLength;
  bool mUrlValidation;

  std::string mOriginalString;
  std::string mReplacedString;


public:

  StringPatch(
    std::vector <uint64_t> offsets,
    std::string name,
    size_t maxLength,
    bool urlValidation,
    std::string original,
    std::string replaced )
    : mOffsets(std::move(offsets))
    , mStringName(std::move(name))
    , mMaxLength(maxLength)
    , mUrlValidation(urlValidation)
    , mOriginalString(std::move(original))
    , mReplacedString(std::move(replaced))
  {}

  void prompt() override
  {
    if ( mStringName.empty() == true )
      return;

    auto maxLen = mMaxLength;

    PromptText( mReplacedString,
      [maxLen, checkUrl = mUrlValidation] ( const std::string& str )
      {
        if ( checkUrl == true )
          return CheckUrl(str, maxLen);

        return CheckStringLength(str, maxLen);
      },
      mStringName );
  }

  void apply( std::fstream& file, OperationMode mode ) const override
  {
    for ( auto offset : mOffsets )
    {
//      clear with zeroes first
      Apply(
        file, offset,
        ByteSequence (mMaxLength, 0) );

      Apply(
        file, mode, offset,
        ToBytes(mOriginalString),
        ToBytes(mReplacedString) );
    }
  }

  static bool CheckStringLength(
  const std::string& text, size_t maxLength );

  static bool CheckUrl(
    const std::string& url, size_t maxLength );
};


inline
bool
StringPatch::CheckStringLength(
  const std::string& text,
  size_t maxLength )
{
  if ( text.size() > maxLength )
  {
    std::cout << "ERROR: String can't be longer than "
              << maxLength << " characters\n";

    return false;
  }

  return true;
}

inline
bool
StringPatch::CheckUrl(
  const std::string& url,
  size_t maxLength )
{
  if ( url.empty() == true )
  {
    std::cout << "ERROR: url can't be empty\n";
    return false;
  }

  if ( url.size() > maxLength )
  {
    std::cout << "ERROR: url can't be longer than "
              << maxLength << " characters\n";

    return false;
  }

  for ( auto& ch : url )
  {
//    Requirements for fully qualified domain name
//    https://datatracker.ietf.org/doc/html/rfc952

    if ( ch >= 'A' && ch <= 'Z' )
      continue;

    if ( ch >= 'a' && ch <= 'z' )
      continue;

    if ( ch >= '0' && ch <= '9' )
      continue;

    if ( ch == '-' || ch == '.' )
      continue;

    std::cout <<
      "ERROR: Invalid character '" << ch << "'. URL may only contain"
      "       letters (a-z, A-Z), digits (0-9), minus signs (-), and periods (.)\n";
    return false;
  }

  if ( url.back() == '.' )
  {
    std::cout << "ERROR: URL isn't allowed to end with a period (.)\n";
    return false;
  }

  if ( url.back() == '-' )
  {
    std::cout << "ERROR: URL isn't allowed to end with a minus sign (-)\n";
    return false;
  }

  return true;
}
