#pragma once


enum ResultCode : int
{
  Success,
  ReadFileFailed,
  WriteFileFailed,
  InvalidExecutableSize,
  PatchFailed,
};
