#pragma once

#include "PatchGroup.hpp"

#include <cstddef>
#include <string>


struct PatchFile
{
  std::string executableName;
  size_t executableSize {};
  std::vector <PatchGroup> groups;
};
