#ifndef LAGER_DEFINES
#define LAGER_DEFINES

// CHP port offsets
const unsigned int CHP_SNAPSHOT_OFFSET = 0;
const unsigned int CHP_PUBLISHER_OFFSET = 1;
const unsigned int CHP_COLLECTOR_OFFSET = 2;
const unsigned int FORWARDER_FRONTEND_OFFSET = 10;
const unsigned int FORWARDER_BACKEND_OFFSET = 11;

// Data Format sizes
const unsigned int UUID_SIZE_BYTES = 16;
const unsigned int VERSION_SIZE_BYTES = 8;
const unsigned int COMPRESSION_SIZE_BYTES = 4;
const unsigned int TIMESTAMP_SIZE_BYTES = 8;

// other
const int BASEPORT_MAX = 65535;
const unsigned int THREAD_CLOSE_WAIT_MILLIS = 100;
const unsigned int THREAD_CLOSE_WAIT_RETRIES = 10;

#endif
