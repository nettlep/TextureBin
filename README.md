# Legacy Code Resurrection Project

This file is part of a glorious rescue mission: migrating ancient code from the dusty vaults of the internet to the shiny halls of GitHub.

This code has been preserved as faithfully as possible, warts and all. All code contained herein is original code written by Paul Nettle unless otherwise noted.

---

## Original description

Texturebin is a high-quality software renderer used to showcase materials. It does this by applying those materials to various objects and rendering them with lighting and shadows, complete with anti-aliasing.

---

## Original Readme.txt

    ------------------------------------------------------------------------------
    Linux build:
    ------------------------------------------------------------------------------

      (note that a pre-built binary already exists)

      1. Make sure libjpg is installed on your box
      2. run make

    ------------------------------------------------------------------------------
    Windows build:
    ------------------------------------------------------------------------------

      (note that a pre-built binary already exists)

      1. You need libjpg. I've precompiled it and placed it in the 'jpeg' folder.
         If you want to update the lib or recompile it, look in the jpeg/source
         folder (I've even included instructions on exactly what I did to generate
         the pre-built libs to save you time.)
      2. Just load 'er up in .NET :)

    ------------------------------------------------------------------------------
    Running it for the first time (both OS's):
    ------------------------------------------------------------------------------

    To run it on the test suite, do this:

      ./texturebin test-images

    This command will tell it to render each of the jpeg files in that folder. The
    default scene file will be used (for defaults see 'usage'). The resulting
    renders will also be placed in test-images, with "-rendered" appended to their
    filenames.

    When it finishes, look in test-images, and all the "-rendered" files will be
    the ones it generated.

    ------------------------------------------------------------------------------
    Usage (both OS's):
    ------------------------------------------------------------------------------

    For usage, just run texturebin without any parms. That's all the documentation
    there is. :)

---

## What's in here?

This repository includes:

- Original source code with minimal modifications  
- Historical comments and design choices, preserved for posterity  
- A fresh coat of licensing and documentation (hello `LICENSE` and `README.md`!)  
- Possibly some delightful quirks from a bygone programming era

---

## License

The code is now released under the [MIT License](LICENSE), unless stated otherwise. You are free to use, modify, and redistribute it.

Note: Original copyright notices from the author have been retained for historical context.

---

## Disclaimer

> This code is **vintage**. That means:
> 
> - Expect odd formatting, outdated conventions, and maybe even some nostalgia.
> - It might not compile or run without some TLC.
> - There's zero warranty, and it may bite. You’ve been warned.

---

## Why preserve this?

Because history matters. This code is a snapshot of how things were done back then—and in some cases, how they’re still done today. Think of it as open source time travel.

Happy coding!

— Paul
