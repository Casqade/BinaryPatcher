#include "Patch.hpp"

#include <iostream>


class MessagePatch : public Patch
{
  std::string mText;

public:

  explicit MessagePatch(
    std::string text )
    : mText(std::move(text))
  {}

  void prompt() override
  {
    std::cout << mText << "\n\n";
  }
};
