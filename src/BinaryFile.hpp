#pragma once

#include "ResultCode.hpp"

#include <fstream>
#include <iostream>


struct BinaryFile
{
  enum OpenModeFlags : int
  {
    Read = 0b01,
    Write = 0b10,
    Truncate = 0b100,
  };

  std::string name {};
  int flags {};

  std::fstream file {};


  ResultCode open();
  void close();

  ResultCode verifySize( size_t expectedSize, bool keepOpen = false );
};

ResultCode
BinaryFile::open()
{
  std::ios::openmode mode = std::ios::binary;
  std::string modeStr {};

  if ( flags & OpenModeFlags::Read )
  {
    mode |= std::ios::in;
    modeStr += "reading";
  }

  if ( flags & OpenModeFlags::Write )
  {
    mode |= std::ios::out;

    if ( flags & OpenModeFlags::Read )
      modeStr += " & ";

    modeStr += "writing";

    if ( flags & OpenModeFlags::Truncate )
    {
      mode |= std::ios::trunc;
      modeStr += "(truncate)";
    }
  }


  if ( file.is_open() == true )
    close();

  std::cout << "Opening '" << name << "'...";

  file.open(name, mode);


  if ( file.is_open() == false )
  {
    if ( modeStr.empty() == false )
      modeStr = " for " + modeStr;

    std::cout << "Failure\n";
    std::cout << "ERROR: Failed to open '" + name + "'" + modeStr << "\n\n";
    return ResultCode::ReadFileFailed;
  }

  file.exceptions(
    file.exceptions() |
    std::fstream::failbit );

  std::cout << "Success\n";

  return ResultCode::Success;
}

void
BinaryFile::close()
{
  file = {};
  flags = {};
}


inline
ResultCode
BinaryFile::verifySize(
  const size_t expectedSize,
  bool keepFileOpen )
{
  flags = BinaryFile::Read;

  if (  auto result = this->open();
        result != ResultCode::Success )
    return result;


  std::cout << "Verifying '" << name << "' size...";

  std::ios::pos_type actualSize {};

  try
  {
    actualSize = file.tellg();
    file.seekg(0, std::ios::end);
    actualSize = file.tellg() - actualSize;
  }
  catch ( const std::exception& e )
  {
    this->close();
    std::cout << "Failure\n";
    std::cout << "ERROR: " << e.what() << "\n\n";
    return ResultCode::ReadFileFailed;
  }

  if ( keepFileOpen == false )
    this->close();


  if ( actualSize != static_cast <std::ios::pos_type> (expectedSize) )
  {
    std::cout << "Failure\n";
    std::cout << "ERROR: this program expects target binary size of "
              << expectedSize << " bytes. '"
              << name + "' is " << actualSize << " bytes\n\n";
    return ResultCode::InvalidExecutableSize;
  }

  std::cout << "Success\n\n";

  return ResultCode::Success;
}
