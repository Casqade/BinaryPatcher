#pragma once

#include "Patch.hpp"

#include <memory>
#include <string>
#include <vector>


struct PatchGroup
{
  std::string name;
  std::string description;
  std::vector <std::unique_ptr <Patch>> entries;
  Patch::OperationMode operationMode {};


  void prompt( const Patch::OperationModeOrder& modeOrder )
  {
    if ( entries.empty() == true )
      return;

    size_t headerLength =
      name.size() + strlen("---  ---");

    std::string headerLine(headerLength, '-');

    std::string fullDescription =
      headerLine + "\n" +
      "--- " + name + " ---\n" +
      headerLine + "\n" +
      description;

    operationMode =
      Patch::PromptOperationMode(fullDescription, modeOrder);

    if ( operationMode != Patch::OperationMode::Modify )
      return;

    for ( auto& entry : entries )
      entry->prompt();
  }

  void apply( std::fstream& file ) const
  {
    if ( operationMode == Patch::OperationMode::Skip )
      return;

    for ( const auto& entry : entries )
      entry->apply(file, operationMode);
  }
};
