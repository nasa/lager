Performance
-----------
Lager includes a couple of different performance measurement utilities.  One is Google's micro-benchmark tool called `benchmark`.  The other is [LTTng](https://lttng.org/).

## Benchmarks
The benchmarks included with Lager are built and run with the target `run_benchmarks` when the project is built in `Release` mode.  To build and run, do the following:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make run_benchmarks
```

## LTTng
The LTTng library enables detailed profiling of Lager's performance.  This is not built by default and requires additional dependencies.

### Dependencies
In addition to the normal Lager dependencies outlined in the README.md:   
[LTTng](https://lttng.org/) - [Ubuntu installation instructions](https://lttng.org/docs/v2.10/#doc-ubuntu)   
Babletrace - [Installation instructions](http://diamon.org/babeltrace/#getting)   
   
### Build
To build Lager with LTTng support, do the following:
```
mkdir build
cd build
cmake -DBUILD_LTTNG=ON ..
make
```

### Tracing
To run a LTTng trace on Lager, you'll need to first prepare a tracing session with the following:
```
lttng-sessiond --daemonize
lttng create test-session-name
lttng enable-event --userspace lager_tp:lager_tp_general
lttng enable-event --userspace lager_tp:lager_tp_zmq_msg
lttng start
```
Then, run the Lager test programs as outlined in the README.md   
   
After the run, stop the trace and generate the output:
```
babeltrace ~/lttng-traces/test-session-name* > lager_trace_output
```
You may view this output raw, or use the included Python script to approximate the throughput for your trace:
```
../scripts/tp_extract_throughput.py lager_trace_output
```
