# BinaryPatcher

[![Windows (MSVC)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msvc.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msvc.yml)
[![Windows (Clang)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-clang.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-clang.yml)
[![Windows (MSYS2)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msys2.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/windows-build-msys2.yml)
[![Ubuntu](https://github.com/Casqade/BinaryPatcher/actions/workflows/ubuntu-build.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/ubuntu-build.yml)
[![macOS](https://github.com/Casqade/BinaryPatcher/actions/workflows/macos-build.yml/badge.svg)](https://github.com/Casqade/BinaryPatcher/actions/workflows/macos-build.yml)
[![GitHub Releases](https://img.shields.io/github/release/Casqade/BinaryPatcher.svg)](https://github.com/Casqade/BinaryPatcher/releases/latest)

**A simple cross-platform tool that empowers users 
to patch and restore binary files (e.g. executables) 
using plain-text configuration files.**

BinaryPatcher supports several patch types which
can replace [number, string, raw byte sequence or flip bit flags](reference.patch).
Users can interactively tweak replaced data thus creating their own unique patch,
which simply can't be done with a pre-patched binary file.

This tool eliminates the technical barrier for end users and provides
the flexibility to apply/remove patches as needed easily.
No programming, no recompiling – just drag,
drop, and follow the interactive prompts.

For devs it's easier to publish & maintain patches. Releasing a new
patch becomes as easy as adding it to the .patch file. 


# How to use

*NOTE: Due to [this annoying Windows UAC issue](https://stackoverflow.com/questions/9190962),
the compiled executable for Windows is distributed as
`BinaryP4tcher.exe` to avoid unnecessary elevation prompts.
To avoid confusion, in this README it will be referenced as `BinaryPatcher.exe`.*

These steps demonstrate how to use BinaryPatcher on BF1942:

1. Download `BinaryPatcher.exe` from the [Releases](https://github.com/Casqade/BinaryPatcher/releases) section
2. Download the .patch file created for your target binary (for BF1942.exe it's `BF1942.patch`)
3. Drag and drop .patch file onto BinaryPatcher.exe
4. Enter the file path to your target binary file (e.g. `D:\Battlefield1942\BF1942.exe`)
5. Follow the prompts
6. To change or restore your file, repeat the procedure STEP3 - STEP5

For patch updates, check back on the repository
or follow your .patch file author announcements.

Patch file extension is irrelevant, .patch is chosen as the default.

You can do any or all of the following:
- Associate BinaryPatcher with whatever extension you choose for its description files,
then simply double-click such file to process it with BinaryPatcher
- Drag & drop a patch file onto BinaryPatcher.exe and it will parse it automatically
- Execute BinaryPatcher and it will ask for a path to patch file


# How it works

- Parses human-readable [patch description list](reference.patch), which defines groups of patches for a specific binary file
- Finds target binary file by name and verifies its size (also specified in patch description)
- Iterates over all patch groups and prompts user for action:
  - Skip patch (don't write to binary)
  - Undo patch (restore original/default bytes)
  - Apply patch
- Prompts user for custom value(s) (strings and numbers) if a patch requires it
- Executes selected actions on patch groups


# How to make patches

To write your own patches,
see [reference.patch](reference.patch)
for supported patch types & syntax.

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

Patch version can be exposed as a part of its filename or
as a comment inside the patch itself.

My library of patches for BF1942 can be found
[here](https://github.com/Casqade/BF1942_patcher/tree/patch-library)


# About

This tool is a direct successor to [BF1942_patcher](https://github.com/Casqade/BF1942_patcher/tree/main),
which feels & behaves exactly the same.

BinaryPatcher is more universal and flexible since editing the patches doesn't require rebuilding the tool.
Besides, human-readable format makes patch descriptions easier to read than C++ spaghetti.
