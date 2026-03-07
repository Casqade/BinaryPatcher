# BinaryPatcher

[![Windows (MSVC)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msvc.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msvc.yml)
[![Windows (Clang)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-clang.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-clang.yml)
[![Windows (MSYS2)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msys2.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msys2.yml)
[![GitHub Releases](https://img.shields.io/github/release/Casqade/BinaryPatcher.svg)](https://github.com/Casqade/BinaryPatcher/releases/latest)

Interactively manages binary file patches defined in a [text-based format](reference.patch):

```
myFile.exe 123456 // file size

Patch
# patch description
{
  binary // patch type
    0x123456 // offset where to apply
      AB CD EF // original bytes
      12 34 56 // replaced bytes
}
```

# How it works

- Parses human-readable [patch description list](reference.patch), which defines groups of patches for a specific binary file
- Finds target binary file by name and verifies its size (also specified in patch description)
- Iterates over all patch groups and prompts user for action:
  - Skip patch (don't write to binary)
  - Undo patch (restore original/default bytes)
  - Apply patch
- Prompts user for custom value(s) (strings and numbers) if a patch requires it
- Executes selected actions on patch groups


# Usage

Download BinaryPatcher.exe from the [Releases](https://github.com/Casqade/BinaryPatcher/releases) section.

Patch description list file can be of any extension.
You can do any or all of the following:
- Associate BinaryPatcher with whatever extension you choose for its description files,
then simply double-click such file to process it with BinaryPatcher
- Drag & drop a patch description file onto BinaryPatcher.exe and it will parse it automatically
- Execute BinaryPatcher and it will ask for a path to patch description file

If you want to write your own patches,
see [reference.patch](reference.patch)
for supported patch types & syntax.

My library of patches for BF1942 can be found
[here](https://github.com/Casqade/BF1942_patcher/tree/patch-library)


# About

This tool is a direct successor to [BF1942_patcher](https://github.com/Casqade/BF1942_patcher/tree/main),
which feels & behaves exactly the same.

BinaryPatcher is more universal and flexible since editing the patches doesn't require rebuilding the tool.
Besides, human-readable format makes patch descriptions easier to read than C++ spaghetti.
