# ping-cli
A simple Linux CLI app that sends an ICMP Echo-Request to a specified destination.

Build
-------

To build ping-cli you will need [CMake](https://cmake.org/) (this code was tested with CMake version 3.10.2) and C++11.
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
 $ sudo ./bin/ping-cli hostname|ip-address
```

Note that you may need to use `sudo` as we are going to open raw sockets. You can specify the destination to ping by either
providing its `hostname` or `IP address`.

Testing
-------

There are two simple tests added to the `ping-cli`: to check for memory leaks and to check that program gracefully terminates. They can be run as follows:

```bash
 $ cd build
 $ make test
```

Authors
-------
- Sergey Madaminov <smadaminov@cs.stonybrook.edu>
