#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "primitives.h"
#include "common.h"
#include "tiny.h"

/* GAPS channel configuration will be automatically linked */
pirate_channel_desc_t pirate_channels[] = {
    PIRATE_CHANNEL_CONFIG(HIGH_TO_LOW_CH, O_RDONLY, LOW_NAME"<-"HIGH_NAME),
    PIRATE_CHANNEL_CONFIG(LOW_TO_HIGH_CH, O_WRONLY, LOW_NAME"->"HIGH_NAME),
    PIRATE_END_CHANNEL_CONFIG
};

struct {
    example_data_t data;            // received data
} ctx;


static int get_data(example_data_t* data) {
    /* Low side requests data by writing zero */
    int len = 0;
    ssize_t num = pirate_write(LOW_TO_HIGH_CH, &len, sizeof(int));
    if (num != sizeof(int)) {
        fprintf(stderr, "Failed to send request\n");
        return -1;
    }
    printf("Sent read request to the %s side\n", HIGH_NAME);

    /* Read and validate response length */
    num = pirate_read(HIGH_TO_LOW_CH, &len, sizeof(len));
    if (num != sizeof(len)) {
        fprintf(stderr, "Failed to receive response length\n");
        return -1;
    }

    if (len >= DATA_LEN) {
        fprintf(stderr, "Response length %d is too large\n", len);
        return -1;
    }

    /* Read back the response */
    num = pirate_read(HIGH_TO_LOW_CH, data->buf, len);
    if (num != len) {
        fprintf(stderr, "Failed to read back the response\n");
        return -1;
    }

    /* Success */
    data->len = len;
    printf("Received %d bytes from the %s side\n\n\n\n\n", data->len, HIGH_NAME);
    return 0;
}

int main(int argc, char* argv[])
{
    server_t si;
    client_t ci;
    request_t ri;
    short port;

    /* Validate and parse command-line options */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[1]);
    printf("\n%s web server on port %d\n\n", LOW_NAME, port);

    /* create, initialize, bind, listen on server socket */
    server_connect(&si, port);

    /*
     * wait for a connection request, parse HTTP, serve high requested content,
     * close connection.
     */
    while (1) {
        /* accept client's connection and open fstream */
        client_connect(&si, &ci);

        /* process client request */
        client_request_info(&ci, &ri);

        /* tiny only supports the GET method */
        if (strcasecmp(ri.method, "GET")) {
            cerror(ci.stream, ri.method, "501", "Not Implemented",
                    "This method is not implemented");
            client_disconnect(&ci);
            continue;
        }

        /* Get data from the high side */
        if (get_data(&ctx.data) != 0) {
            cerror(ci.stream, ri.method, "501", "No data",
                    "Failed to fetch GAPS data");
            client_disconnect(&ci);
            continue;
        }

        /* Serve low content */
        int ret = serve_static_content(&ci, &ri, ctx.data.buf, ctx.data.len);
        if  (ret < 0) {
            client_disconnect(&ci);
            continue;
        }

        client_disconnect(&ci);
    }

    return 0;
}
