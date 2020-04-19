# ping-cli
A simple CLI app that sends an ICMP Echo-Request to a specified destination.

Build
-------

To build ping-cli you will need [CMake](https://cmake.org/) (this code was tested with CMake version 3.10.2) as well as C and C++ compilers.
To build ping-cli run the following commands in root directory of the repository:

```bash
 $ mkdir build
 $ cd build
 $ cmake ..
 $ make
 $ cd ..
```

Run ping-cli
-------

To run ping-cli run the following in the root directory of the repository:

```bash
 $ ./bin/ping-cli
```

Authors
-------
- Sergey Madaminov <smadaminov@cs.stonybrook.edu>
