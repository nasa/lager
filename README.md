Lager
-----------

Light-weight Accumulator Gathering Efficiently in Real-time   
   
Lager is a light-weight logging system.

### Dependencies Linux

`sudo apt install -y git cmake build-essential libzmq3-dev uuid-dev`   

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

Open a terminal window and run the server   
   
`./server`   
   
Open a second terminal window and run the client   
   
`./client`   
   
You will see some output showing the hashmap being sent, and the client sending an update.  You may update the `client_main.cpp` and run additional clients to play around with it.
=======
Lager is a light-weight logging system.
>>>>>>> 89625b53f1073b2aa26becd08e9f7d4b13f086a3
