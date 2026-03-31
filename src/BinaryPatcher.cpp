///
/// Building:
///
/// cl /std:c++17 /EHsc BinaryPatcher.cpp && del BinaryPatcher.obj
///
/// g++ --std=c++17 -static BinaryPatcher.cpp -o BinaryPatcher.exe
///

#include "PatchFileParser.hpp"
#include "BinaryFile.hpp"


static
void
waitForUserExit()
{
  std::cout << "Press ENTER to exit...\n";
  std::cin.get();
}


int
main(
  int argc,
  char** argv )
{
  std::cout << "BinaryPatcher v1.0 by casqade\n"
    "  source: https://github.com/Casqade/BinaryPatcher\n\n";

  std::string patchFilePath;

  if ( argc > 1 )
  {
    patchFilePath = argv[1];
  }
  else
  {
    std::cout << "Enter path to patch file: ";
    std::getline(std::cin, patchFilePath);
    std::cout << "\n";
  }


  std::cout << "Parsing patch file '" + patchFilePath + "'\n\n";

  std::ifstream file(patchFilePath);

  if ( file.is_open() == false )
  {
    std::cout << "ERROR: Failed to open patch file '" << patchFilePath << "'\n\n";
    waitForUserExit();
    return ResultCode::ReadFileFailed;
  }


  PatchFile patchFile;
  PatchFileParser parser;

  try
  {
    patchFile = parser.parse(file);
  }
  catch ( const std::exception& e )
  {
    std::cout << "ERROR: Failed to parse patch file '" << patchFilePath << "': " << e.what() << "\n\n";
    waitForUserExit();
    return ResultCode::ReadFileFailed;
  }

  file.close();


  std::cout << "Enter path to " << patchFile.executableName
            << " or leave empty to use default ("
            << patchFile.executableName << "): ";

  std::string exePath;
  std::getline(std::cin, exePath);

  if ( exePath.empty() == true )
    exePath = patchFile.executableName;

  std::cout << "\n";


  BinaryFile executable
  {
    exePath,
  };

  if (  auto result = executable.verifySize(patchFile.executableSize);
        result != ResultCode::Success )
  {
    waitForUserExit();
    return result;
  }


  for ( auto& group : patchFile.groups )
    group.prompt(parser.operationOrder());


  executable.flags = BinaryFile::Read | BinaryFile::Write;

  if (  auto result = executable.open();
        result != ResultCode::Success )
  {
    waitForUserExit();
    return result;
  }

  ResultCode result {};

  try
  {
    std::cout << "Patching '" << executable.name << "'...";

    for ( const auto& group : patchFile.groups )
      group.apply(executable.file);

    std::cout << "Success\n\n";
  }
  catch ( const std::exception& e )
  {
    std::cout << "Failure\n";
    std::cout << "ERROR: " << e.what() << "\n\n";
    result = ResultCode::PatchFailed;
  }

  executable.close();

  waitForUserExit();

  return result;
}
