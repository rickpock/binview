# BinView

BinView is a library to interpret the contents of binary files for common file types.

BinView represents the file contents hierarchically. The entire file is the root of the tree, and sections can be broken down recursively.

This is currently in a prototype stage, meaning that the architecture is being explored and subject to change at any time.

# Vision

The project is planned as a library, allowing it to be used in any hex viewer or IDE. The file parsing is intended to be extensible, allowing multiple contributors to add their own file parsers.

## Features Planned for Version 1.0

- Hierarchical File Format definition and parser. The Hierarchical File Format allows new file structures to be defined as metadata. BinView will be able to read .hff files to understand how to parse a given file.
- Two HFF definitions for common file types. Examples: .zip, .jpg, ELF, .wav.
- Basic Hints engine. The hints engine will use a heuristic to determine which HFF definition should be used to parse a file. This may be based on the file extension, context, file signature (first N bytes), or a partial parsing of the file.
- API for library consumers. BinView is a library intended to be used by plugins for existing hex viewers and IDEs. The API defines publicly available methods to use BinView.
- Minimal test consumer. A simple consumer for the library that can handle visualizing data for an arbitrarily large file.

## Non-Features for Version 1.0

These are future features for BinView on the roadmap, but not for version 1.0.

- Decompression or decryption. For example, BinView 1.0 will show the compressed data of a .zip file as hex values, but will not decompress the data.
- Recursive HFF structure. BinView 1.0 will not allow for HFF's referencing other HFF's to interpret partial file data.
- Stream-based parsing. BinView 1.0 will require random access (seeking) to the file being parsed.
- Advanced Hints engine. An extension of the hints engine to support recursive HFFs.
- Efficient file access. BinView 1.0 will provide file parsing, but not necessarily _efficient_ file parsing.

# License

This project is licensed under the MIT License.
