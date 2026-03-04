#include "Patch.hpp"

#include <cstdint>
#include <vector>
#include <ios>


class FlagsPatch : public Patch
{
  std::vector <uint64_t> mOffsets;

  uint64_t mMask;
  size_t mByteCount;


public:

  FlagsPatch(
    std::vector <uint64_t> offsets,
    uint64_t mask,
    size_t byteCount )
    : mOffsets(std::move(offsets))
    , mMask(mask)
    , mByteCount(byteCount)
  {}

  void apply( std::fstream& file, OperationMode mode ) const override
  {
    for ( auto offset : mOffsets )
    {
      uint64_t value = 0;

//      TODO: ensure
//        offset + sizeof(value) < executable size

      ReadBytes(
        file, offset,
        reinterpret_cast <char*> (&value),
        mByteCount );

      switch (mode)
      {
        case OperationMode::Modify:
          value |= mMask;
          break;

        case OperationMode::Restore:
          value &= ~mMask;
          break;

        default:
          break;
      }

      Apply(
        file, offset,
        ToBytes(value, mByteCount) );
    }
  }
};
