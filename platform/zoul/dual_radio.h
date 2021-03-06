#include "contiki-conf.h"
#include "cc1200.h"
#include "cpu/cc2538/dev/cc2538-rf.h"
#include "net/rpl/rpl-private.h"

#ifndef SHORT_RADIO
#define SHORT_RADIO 2
#endif

#ifndef LONG_RADIO
#define LONG_RADIO 1
#endif

#ifndef BOTH_RADIO
#define BOTH_RADIO 0
#endif

#ifndef DUAL_DUTY_RATIO
#define DUAL_DUTY_RATIO 1
#endif


/* Radio drivers */
#if DUAL_RADIO
extern struct radio_driver NETSTACK_CONF_RADIO;
extern struct radio_driver NETSTACK_RADIO;
#endif

/* Functions */
int dual_radio_switch(int radio);
int dual_radio_change(void);
int dual_radio_received(int radio);
int radio_received_is_longrange(void);
int sending_in_LR(void);
int dual_radio_turn_on(char targetRadio);
int dual_radio_turn_off(char targetRadio);

/* Process */
int dio_broadcast(rpl_instance_t* instance);
int dis_broadcast(void);

/* Global variable for Dual RPL */
int long_range_radio;
int radio_received;
char dual_duty_cycle_count;

/* Global variable for Dual radio tcpip */
uint8_t packet_forwarding;
