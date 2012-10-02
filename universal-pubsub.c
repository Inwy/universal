/* This software is the core component of the ND-OV system
 * each multipart message it receives from a transport operator
 * consisting of an envelope and its data, is broadcasted to
 * all connected serviceproviders.
 *
 * Requirements: zeromq2
 * gcc -lzmq -o universal-pubsub universal-pubsub.c
 *
 * Changes:
 *  - Initial version <stefan@opengeo.nl>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <zmq.h>

int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("%s [receiver] [pubsub]\n\nEx.\n" \
                "%s tcp://127.0.0.1:7807 tcp://127.0.0.1:7817\n",
                argv[0], argv[0]);
        exit(-1);
    }

    void *context  = zmq_init (1);
    void *pubsub   = zmq_socket (context, ZMQ_PUB);
    void *receiver = zmq_socket (context, ZMQ_PULL);

    zmq_bind (pubsub,   argv[2]);
    zmq_bind (receiver, argv[1]);

    while (1) {
        int64_t more;
        size_t more_size = sizeof more;
        do {
            /* Create an empty 0MQ message to hold the message part */
            zmq_msg_t part;
            int rc = zmq_msg_init (&part);
            assert (rc == 0);
            /* Block until a message is available to be received from the socket */
            rc = zmq_recv (receiver, &part, 0);
            assert (rc == 0);
            /* Determine if more message parts are to follow */
            rc = zmq_getsockopt (receiver, ZMQ_RCVMORE, &more, &more_size);
            assert (rc == 0);
            if (more) {
                zmq_send (pubsub, &part, ZMQ_SNDMORE);
            } else {
                zmq_send (pubsub, &part, 0);
            }
            zmq_msg_close (&part);
        } while (more);
    }
}
