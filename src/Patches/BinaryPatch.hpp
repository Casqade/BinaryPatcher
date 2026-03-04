#pragma once

#include "Patch.hpp"

#include <cstdint>
#include <vector>
#include <ios>


class BinaryPatch : public Patch
{
  std::vector <uint64_t> mOffsets;
  ByteSequence mOriginalBytes;
  ByteSequence mModifiedBytes;

public:

  BinaryPatch(
    std::vector <uint64_t> offsets,
    ByteSequence original,
    ByteSequence modified )
    : mOffsets(std::move(offsets))
    , mOriginalBytes(std::move(original))
    , mModifiedBytes(std::move(modified))
  {}

  void apply( std::fstream& file, OperationMode mode ) const override
  {
    for ( auto offset : mOffsets )
    {
      Apply(
        file, mode, offset,
        mOriginalBytes,
        mModifiedBytes );
    }
  }
};

