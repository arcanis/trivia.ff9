# Final Fantasy IX :: Binmaster

This project aims to create extracting tools for Final Fantasy IX.

Please check out the [wiki](https://github.com/arcanis/trivia.ff9/wiki/_pages) !

## Installation

### Dependencies

We're using Boost (filesystem, system & spirit) and CMake/Make.

You will have to use your own FF9.IMG file, we won't provide any.

### Compilation

    $> git clone git@github.com:arcanis/trivia.ff9 ff9binmaster
    $> cd ff9binmaster/build
    $> cmake .. && make

## Usage

### unpack

    $> unpack "<FF9.IMG path>" "<destination folder>"

This utility will unpack the FF9.IMG file inside the destination folder. The unpacked file can then be read by the other tools (which does not currently exists ;).

## Help

We're needing more people ! If you know anything about the game structure, please share it so we can build better tools together !

If you find an issue in one of the utilities, or want to discuss about a wiki article, please feel free to open an [issue](https://github.com/arcanis/trivia.ff9/issues).

## Credits

The project is lead by [Maël Nison](http://www.arcastel.com).

Multiple informations from this repository come from the [Qhimm](http://www.qhimm.com) community. Thanks !
