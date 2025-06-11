#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <modbus.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
// Constants
#define SERVER_IP "192.168.1.250"
#define SERVER_PORT 502
#define NUM_DI 18
// Global Variables
const uint16_t COIL_ADDRESS = 33; // Address for a single coil
uint8_t di_values[NUM_DI];
uint8_t previous_di[NUM_DI];
modbus_t* ctx = NULL;
pthread_mutex_t modbus_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread
synchronization
// Function Prototypes
void* read_di_values(void* arg);
void* do_thread(void* arg);
void reconnect_modbus();
int main() {pthread_t thread1, thread2;
// Initialize Modbus TCP
ctx = modbus_new_tcp(SERVER_IP, SERVER_PORT);
if (ctx == NULL) {
fprintf(stderr, "Unable to create Modbus context: %s\n", modbus_strerror(errno));
return -1;
}
if (modbus_connect(ctx) == -1) {
fprintf(stderr, "Unable to connect to Modbus server: %s\n", modbus_strerror(errno));
modbus_free(ctx);
return -1;
}
printf("Connected to Modbus server.\n");
// Create threads for DI and DO operations
if (pthread_create(&thread1, NULL, read_di_values, NULL) != 0) {
fprintf(stderr, "Failed to create DI thread\n");
modbus_free(ctx);
return -1;
}
if (pthread_create(&thread2, NULL, do_thread, NULL) != 0) {
fprintf(stderr, "Failed to create DO thread\n");
modbus_free(ctx);
return -1;
}
// Wait for threads to complete
pthread_join(thread1, NULL);
pthread_join(thread2, NULL);
// Cleanup
modbus_close(ctx);
modbus_free(ctx);
pthread_mutex_destroy(&modbus_mutex);
return 0;}
// Function to reconnect Modbus if connection is lost
void reconnect_modbus() {
fprintf(stderr, "Reconnecting to Modbus server...\n");
modbus_close(ctx);
modbus_free(ctx);
ctx = modbus_new_tcp(SERVER_IP, SERVER_PORT);
if (ctx == NULL) {
fprintf(stderr, "Failed to recreate Modbus context\n");
exit(EXIT_FAILURE);
}
if (modbus_connect(ctx) == -1) {
fprintf(stderr, "Failed to reconnect to Modbus server: %s\n", modbus_strerror(errno));
exit(EXIT_FAILURE);
}
printf("Reconnected to Modbus server.\n");
}
// Thread function to continuously read and monitor DI values
void* read_di_values(void* arg) {
static int first_run = 1;
printf("DI thread started.\n");
while (1) {
uint8_t temp_di[NUM_DI] = {0};
pthread_mutex_lock(&modbus_mutex); // Lock the Modbus context
if (ctx == NULL) {
reconnect_modbus();
}
// Read DI values
int rc = modbus_read_input_bits(ctx, 0x0001, NUM_DI, temp_di);
pthread_mutex_unlock(&modbus_mutex); // Unlock after read operation
if (rc == -1) {fprintf(stderr, "Error reading DI values: %s\n", modbus_strerror(errno));
usleep(500000); // Wait before retrying
continue;
}
// Print DI values
printf("DI values: ");
for (int i = 0; i < NUM_DI; i++) {
printf("%d ", temp_di[i]);
}
printf("\n");
// Initialize previous DI values on first run
if (first_run) {
memcpy(previous_di, temp_di, NUM_DI);
first_run = 0;
}
// Detect changes
for (int i = 0; i < NUM_DI; i++) {
if (temp_di[i] != previous_di[i]) {
printf("DI[%d] changed: %d -> %d\n", i + 1, previous_di[i], temp_di[i]);
}
}
// Update previous DI values
memcpy(previous_di, temp_di, NUM_DI);
usleep(500000); // Delay for 500ms
}
return NULL;
}
// Thread function for writing and reading DO values (Coil)
void* do_thread(void* arg) {
printf("DO thread started.\n");
while (1) {
int rc;uint8_t coil_value = 0;
pthread_mutex_lock(&modbus_mutex); // Lock before accessing Modbus
if (ctx == NULL) {
reconnect_modbus();
}
// Toggle coil ON
printf("Setting coil at address 0x00%u to ON.\n", COIL_ADDRESS);
rc = modbus_write_bit(ctx, COIL_ADDRESS, 1);
if (rc == -1) {
fprintf(stderr, "Error writing to coil: %s\n", modbus_strerror(errno));
pthread_mutex_unlock(&modbus_mutex);
usleep(500000);
continue;
}
usleep(200000); // Small delay after writing
// Read back coil value
rc = modbus_read_bits(ctx, COIL_ADDRESS, 1, &coil_value);
if (rc == -1) {
fprintf(stderr, "Error reading coil: %s\n", modbus_strerror(errno));
pthread_mutex_unlock(&modbus_mutex);
usleep(500000);
continue;
}
printf("Coil state at address 0x00%u: %d\n", COIL_ADDRESS, coil_value);
usleep(200000); // Small delay after reading
// Toggle coil OFF
printf("Setting coil at address 0x00%u to OFF.\n", COIL_ADDRESS);
rc = modbus_write_bit(ctx, COIL_ADDRESS, 0);
if (rc == -1) {
fprintf(stderr, "Error writing to coil: %s\n", modbus_strerror(errno));
pthread_mutex_unlock(&modbus_mutex);
usleep(500000);
continue;
}pthread_mutex_unlock(&modbus_mutex); // Unlock after operation
usleep(1000000); // Delay for 1 second
}
return NULL;
}
