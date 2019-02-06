find_package(LTTngUST REQUIRED)

lttngust_gen(LTTNG_SRCS LTTNG_HDRS src/lager.tp)

add_definitions(-DWITH_LTTNG)
