libswiftnav
===========

Libswiftnav (LSN) is a platform independent library that implements GNSS utility functions for use by software-defined GNSS receivers or software requiring GNSS functionality. It is intended to be as portable as possible and is written in standards compliant C with no dependancies

LSN does not provide any functionality for communicating with Swift
Navigation receivers.  See [libsbp](https://github.com/swift-nav/libsbp) to
communicate with receivers using Swift Binary Protocol (SBP).

To checkout the library run the following commands in an appropriate directory
```
git clone git@github.com:swift-nav/libswiftnav.git
```
Which should checkout the source code

To build the library, run the following commands from the LSN root directory - LSN depends on the latest xcode for MacOSX and cmake
```
mkdir ./build
cd ./build
cmake ../
make -j4
```
