Lager
-----------

Light-weight Accumulator Gathering Efficiently in Real-time   
   
Lager is a light-weight logging system.

### Dependencies Linux

Package dependencies   

`sudo apt install -y git cmake build-essential uuid-dev xerces-c-dev`   

[CMake](https://cmake.org) >= 3.1.3 required.  Note this package is `cmake` in Ubuntu 16.04 and `cmake3` in Ubuntu 14.04.  The primary reason for CMake 3.1.3 is XercesC works better with `find_package()`   
[ZeroMQ](https://github.com/zeromq/libzmq) >= 4.2.2 required.  To install from source:   
   
```
cd
git clone https://github.com/zeromq/libzmq
cd libzmq
git checkout v4.2.2
mkdir build
cd build
cmake ..
make
sudo make install
cd
git clone https://github.com/zeromq/cppzmq
cd cppzmq
git checkout v4.2.2
sudo cp zmq.hpp /usr/local/include/
```

### Building Linux

1. `mkdir build`   
2. `cd build`   
3. `cmake ..`   
4. `make`   

### Dependencies Windows

1. Clone [libzmq](https://github.com/zeromq/libzmq) and use build directions located in `./builds/msvc/`.   
2. Clone [cppzmq](https://github.com/zeromq/cppzmq) and copy the `*.hpp` files into your `libzmq/include` directory.   
3. Set an environment variable `ZeroMQ_ROOT_DIR` to the full path to the libzmq directory.

### Building Windows

Replace `Visual Studio 15 2017` with your appropriate version.   
   
1. `mkdir build`   
2. `cd build`   
3. `cmake -G "Visual Studio 15 2017" ..`   
4. `cmake --build .`   

### Usage

Open a terminal window and run the bartender   
   
`./bartender`   
   
Open a second terminal window and run the tap   
   
`./test_tap`   
   
Open a third terminal window and run the mug   
   
`./test_mug`   
