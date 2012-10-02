/* This software is the client component of the ND-OV system
 * each multipart message it receives from the ND-OV system
 * consisting of an envelope and its data, is received and
 * rebroadcasted to all connected clients of the serviceprovider.
 *
 * Requirements: zeromq2
 * gcc -lzmq -o universal-pubsub universal-pubsub.c
 *
 * Changes:
 *  - Initial version <stefan@opengeo.nl>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <zmq.h>

int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("%s [subscriber] [pubsub]\n\nEx.\n" \
                "%s tcp://127.0.0.1:7817 tcp://127.0.0.1:7827\n",
                argv[0], argv[0]);
        exit(-1);
    }

    void *context  = zmq_init (1);
    void *pubsub   = zmq_socket (context, ZMQ_PUB);
    void *subscriber = zmq_socket (context, ZMQ_SUB);

    /* Apply a high water mark at the PubSub */
    uint64_t hwm   = 255;
    zmq_setsockopt(pubsub, ZMQ_HWM, &hwm, sizeof(hwm));

    zmq_bind (pubsub,   argv[2]);
    zmq_connect (subscriber, argv[1]);

    /* Apply the subscriptions */
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "", 0);

    while (1) {
        int64_t more;
        size_t more_size = sizeof more;
        do {
            /* Create an empty 0MQ message to hold the message part */
            zmq_msg_t part;
            int rc = zmq_msg_init (&part);
            assert (rc == 0);
            /* Block until a message is available to be received from the socket */
            rc = zmq_recv (subscriber, &part, 0);
            assert (rc == 0);
            /* Determine if more message parts are to follow */
            rc = zmq_getsockopt (subscriber, ZMQ_RCVMORE, &more, &more_size);
            assert (rc == 0);
            /* Send the message, when more is set, apply the flag, otherwise don't */
            zmq_send (pubsub, &part, (more ? ZMQ_SNDMORE : 0));
            zmq_msg_close (&part);
        } while (more);
    }
}
