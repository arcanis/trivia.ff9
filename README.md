# Final Fantasy IX :: Binmaster

This project aims to create extraction and conversion tools for Final Fantasy IX.

Please check out the [wiki](https://github.com/arcanis/trivia.ff9/wiki/_pages) !

## Installation

### Dependencies

We're using Boost (filesystem, system & spirit) and CMake/Make.

You will have to use your own FF9.IMG file, we won't provide any.

### Compilation

    $> git clone git@github.com:arcanis/trivia.ff9 ff9-utilities
    $> cd ff9-utilities/build
    $> cmake .. && make

## Usage

### ffix-extract-img

    $> ffix-extract-img <FF9.IMG path> <destination folder>

This utility extracts the FF9.IMG directory tree into the specified destination folder. The files can then be read by the other tools of the suite.

### ffix-extract-db

    $> ffix-extract-db <.ff9db path> <destination folder>

This utility extracts the files from the DB file.

**Note** It can happen that a DB file contains other DB files.

### ffix-convert-bs

    $> ffix-convert-bs <.ff9bs path> <destination folder> [--tim <.tim file>, [--tim <.tim file>]] [--fake-textures-extension <.ext>]

This utility converts a FF9 battle scene into an OBJ file. Model textures are also exported in the same pass.

You can (and probably should) specify TIM files which will be loaded into the VRAM. Without this, exported textures will be black.

The exported textures will be in TGA 32-bits file format (even if you specify `--fake-textures-extension`). However, the `--fake-textures-extensions` will change the file extension used in the `materials.mtl` file. It will then be up to you to convert the textures from the TGA files (we advice the `mogrify` utility, from the imagemagick toolset).

**Note** For reference, battle scenes are located in the folder 06 of the extracted image tree.

## Help

We're needing more people ! If you know anything about the game structure, please share it so we can build better tools together !

If you find an issue in one of the utilities, or want to discuss about a wiki article, please feel free to open an [issue](https://github.com/arcanis/trivia.ff9/issues).

## Credits

The project is led by [Maël Nison](http://www.arcastel.com).

A lot of informations from this repository (including the wiki) have been gathered from the [Qhimm](http://www.qhimm.com) community work, especially Zidane_2's utilities. Thanks !
