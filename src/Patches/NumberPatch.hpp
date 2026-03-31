#include "Patch.hpp"
#include "../NumberValue.hpp"

#include <cstdint>
#include <vector>
#include <ios>


class NumberPatch : public Patch
{
  std::vector <uint64_t> mOffsets;

  std::string mNumberName;

  NumberValue mRangeMin;
  NumberValue mRangeMax;

  NumberValue mOriginalValue;
  NumberValue mCurrentValue;


public:

  NumberPatch(
    std::vector <uint64_t> offsets,
    std::string name,
    NumberValue rangeMin,
    NumberValue rangeMax,
    NumberValue original,
    NumberValue current )
    : mOffsets(std::move(offsets))
    , mNumberName(std::move(name))
    , mRangeMin(rangeMin)
    , mRangeMax(rangeMax)
    , mOriginalValue(original)
    , mCurrentValue(current)
  {}

  void prompt() override
  {
    if ( mNumberName.empty() == false )
      PromptNumber(mCurrentValue, mRangeMin, mRangeMax, mNumberName);
  }

  void apply( std::iostream& stream, OperationMode mode ) const override
  {
    for ( auto offset : mOffsets )
    {
      Apply(
        stream, mode, offset,
        mOriginalValue.toBytes(),
        mCurrentValue.toBytes() );
    }
  }
};
