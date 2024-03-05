
#ifndef __UDP_ECHO__H__
#define __UDP_ECHO__H__

#define UDP_DST_IP0 (192)
#define UDP_DST_IP1 (168)
#define UDP_DST_IP2 (123)
#define UDP_DST_IP3 (178)

#define UDP_TEST_PORT (3365)

void udp_echo_init(void);
void udp_echo_raw_init(void);

#endif