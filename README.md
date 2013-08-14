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

    $> ffix-extract-img "<FF9.IMG path>" "<destination folder>"

This utility extracts the FF9.IMG directory tree into the specified destination folder. The files can then be read by the other tools of the suite.

### ffix-extract-db

    $> ffix-extract-db "<*.ff9db path>" "<destination folder>"

This utility extracts the files from the DB file.

**Note** It can happen that a DB file contains other DB files.

## Help

We're needing more people ! If you know anything about the game structure, please share it so we can build better tools together !

If you find an issue in one of the utilities, or want to discuss about a wiki article, please feel free to open an [issue](https://github.com/arcanis/trivia.ff9/issues).

## Credits

The project is led by [Maël Nison](http://www.arcastel.com).

A lot of informations from this repository (including the wiki) have been gathered from the [Qhimm](http://www.qhimm.com) community work, especially Zidane_2's utilities. Thanks !
