/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A simple power saving MAC protocol based on X-MAC [SenSys 2006]
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "dev/leds.h"
#include "dev/radio.h"
#include "dev/watchdog.h"
#include "net/netstack.h"
#include "lib/random.h"
#include "net/mac/rimac/rimac.h"
#include "net/rime/rime.h"
#include "net/rime/timesynch.h"
#include "sys/compower.h"
#include "sys/pt.h"
#include "sys/rtimer.h"

#include "contiki-conf.h"
#include "sys/cc.h"

#ifdef EXPERIMENT_SETUP
#include "experiment-setup.h"
#endif

#include <string.h>

#include "../lanada/param.h"

/*
#if CONTIKI_TARGET_COOJA
#include "lib/simEnvChange.h"
#include "sys/cooja_mt.h"
#endif  CONTIKI_TARGET_COOJA
*/

#ifndef WITH_ACK_OPTIMIZATION
#define WITH_ACK_OPTIMIZATION        0
#endif
#ifndef WITH_ENCOUNTER_OPTIMIZATION
#define WITH_ENCOUNTER_OPTIMIZATION  0
#endif
#ifndef WITH_STREAMING
#define WITH_STREAMING               1
#endif
#ifndef WITH_STROBE_BROADCAST
#define WITH_STROBE_BROADCAST        1
#endif

struct announcement_data {
  uint16_t id;
  uint16_t value;
};

/* The maximum number of announcements in a single announcement
   message - may need to be increased in the future. */
#define ANNOUNCEMENT_MAX 10

/* The structure of the announcement messages. */
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[ANNOUNCEMENT_MAX];
};

/* The length of the header of the announcement message, i.e., the
   "num" field in the struct. */
#define ANNOUNCEMENT_MSG_HEADERLEN (sizeof (uint16_t))

#define DISPATCH          0
#define TYPE_STROBE       0x10
/* #define TYPE_DATA         0x11 */
#define TYPE_ANNOUNCEMENT 0x12
#define TYPE_STROBE_SHORT_ACK   0x13
#define TYPE_STROBE_LONG_ACK   0x23
#define TYPE_STROBE_LONG_BROADCAST_ACK   0x24
#if DATA_ACK
#define TYPE_DATA_ACK   0x14
#endif

struct rimac_hdr {
  uint8_t dispatch;
  uint8_t type;
/*
#if STROBE_CNT_MODE
  uint8_t strobe_cnt; // For strobe cnt mode
#endif
*/
};

#ifdef COOJA

#define MAX_STROBE_SIZE 50

#define CARRIER_SENSING_TIME RTIMER_ARCH_SECOND / 1000 * 2

#ifdef RIMAC_CONF_ON_TIME
#define DEFAULT_ON_TIME (RIMAC_CONF_ON_TIME)
#else
#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 80)
// #define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 58)
#endif

#ifdef RIMAC_CONF_OFF_TIME
#define DEFAULT_OFF_TIME (RIMAC_CONF_OFF_TIME)
#else
// #define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE - DEFAULT_ON_TIME)
#define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE - DEFAULT_ON_TIME)
#endif

#define DEFAULT_PERIOD (DEFAULT_OFF_TIME + DEFAULT_ON_TIME)

#define WAIT_TIME_BEFORE_STROBE_ACK RTIMER_ARCH_SECOND / 1000

/* On some platforms, we may end up with a DEFAULT_PERIOD that is 0
   which will make compilation fail due to a modulo operation in the
   code. To ensure that DEFAULT_PERIOD is greater than zero, we use
   the construct below. */
#if DEFAULT_PERIOD == 0
#undef DEFAULT_PERIOD
#define DEFAULT_PERIOD 1
#endif

/* The cycle time for announcements. */
#define ANNOUNCEMENT_PERIOD 4 * CLOCK_SECOND

/* The time before sending an announcement within one announcement
   cycle. */
#define ANNOUNCEMENT_TIME (random_rand() % (ANNOUNCEMENT_PERIOD))

#define DEFAULT_STROBE_WAIT_TIME (7 * DEFAULT_ON_TIME / 16)
// #define DEFAULT_STROBE_WAIT_TIME (5 * DEFAULT_ON_TIME / 6)

#else /* COOJA */


#define MAX_STROBE_SIZE 50

#define CARRIER_SENSING_TIME RTIMER_ARCH_SECOND / 1000 * 2

#ifdef RIMAC_CONF_ON_TIME
#define DEFAULT_ON_TIME (RIMAC_CONF_ON_TIME)
#else
//#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 80)
#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 56)
#endif

#ifdef RIMAC_CONF_OFF_TIME
#define DEFAULT_OFF_TIME (RIMAC_CONF_OFF_TIME)
#else
// #define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE - DEFAULT_ON_TIME)
#define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE - DEFAULT_ON_TIME)
#endif

#define DEFAULT_PERIOD (DEFAULT_OFF_TIME + DEFAULT_ON_TIME)

#define WAIT_TIME_BEFORE_STROBE_ACK RTIMER_ARCH_SECOND / 1000

/* On some platforms, we may end up with a DEFAULT_PERIOD that is 0
   which will make compilation fail due to a modulo operation in the
   code. To ensure that DEFAULT_PERIOD is greater than zero, we use
   the construct below. */
#if DEFAULT_PERIOD == 0
#undef DEFAULT_PERIOD
#define DEFAULT_PERIOD 1
#endif

/* The cycle time for announcements. */
#define ANNOUNCEMENT_PERIOD 4 * CLOCK_SECOND

/* The time before sending an announcement within one announcement
   cycle. */
#define ANNOUNCEMENT_TIME (random_rand() % (ANNOUNCEMENT_PERIOD))

// #define DEFAULT_STROBE_WAIT_TIME (7 * DEFAULT_ON_TIME / 16)
#define DEFAULT_STROBE_WAIT_TIME (2 * DEFAULT_ON_TIME / 3)
#endif /* COOJA */

struct rimac_config rimac_config = {
  DEFAULT_ON_TIME,
  DEFAULT_OFF_TIME,
/*  Original setting */
//  4 * DEFAULT_ON_TIME + DEFAULT_OFF_TIME,
/*  Customized setting */
  5 * DEFAULT_ON_TIME + DEFAULT_OFF_TIME,
  DEFAULT_STROBE_WAIT_TIME
};

#include <stdio.h>

static struct pt pt;
PROCESS(strobe_wait, "strobe wait");
static volatile unsigned char strobe_target;

static volatile uint8_t rimac_is_on = 0;

static volatile unsigned char waiting_for_packet = 0;
static volatile unsigned char someone_is_sending = 0;
static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;
static volatile unsigned char interference = 0;
static volatile unsigned char backoff = 0;

#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE

#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#define LEDS_TOGGLE(x) leds_toggle(x)
#define DEBUG 0
#define TIMING 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else
/* JOONI
 * Enable LEDS when DEBUG == 0 
#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE
#define LEDS_ON(x)
#define LEDS_OFF(x)
#define LEDS_TOGGLE(x)
*/ 
#define PRINTF(...)
#define PRINTDEBUG(...)
#endif

#if DUAL_RADIO
#ifdef ZOLERTIA_Z1
#include	"../platform/z1/dual_radio.h"
#elif COOJA /* ZOLERTIA_Z1 */
#include	"../platform/cooja/dual_conf.h"
#else /* ZOLERTIA_Z1 */
#include "../platform/zoul/dual_radio.h"
#endif /* ZOLERTIA_Z1 */
#endif /* DUAL_RADIO */
#include "sys/log_message.h"
// extern int rimac_collision_count, rimac_transmission_count;

#if RIMAC_CONF_ANNOUNCEMENTS
/* Timers for keeping track of when to send announcements. */
static struct ctimer announcement_cycle_ctimer, announcement_ctimer;

static int announcement_radio_txpower;
#endif /* RIMAC_CONF_ANNOUNCEMENTS */

/* Flag that is used to keep track of whether or not we are listening
   for announcements from neighbors. */
static uint8_t is_listening;

#if RIMAC_CONF_COMPOWER
static struct compower_activity current_packet;
#endif /* RIMAC_CONF_COMPOWER */

#if WITH_ENCOUNTER_OPTIMIZATION

#include "lib/list.h"
#include "lib/memb.h"

struct encounter {
  struct encounter *next;
  linkaddr_t neighbor;
  rtimer_clock_t time;
};

#define MAX_ENCOUNTERS 4
LIST(encounter_list);
MEMB(encounter_memb, struct encounter, MAX_ENCOUNTERS);
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

static uint8_t is_streaming;
static linkaddr_t is_streaming_to, is_streaming_to_too;
static rtimer_clock_t stream_until;
#define DEFAULT_STREAM_TIME (RTIMER_ARCH_SECOND)

static uint8_t is_short_waiting = 0;

/* remaining energy JJH */
#if RPL_ENERGY_MODE
extern uint8_t remaining_energy;
#endif
#if DUAL_RADIO
static void dual_radio_on(char target);
static void dual_radio_off(char target);
#endif
/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(rimac_is_on && radio_is_on == 0) {
    radio_is_on = 1;
#if DUAL_RADIO
		dual_radio_turn_on(BOTH_RADIO);
#else
    NETSTACK_RADIO.on();
    LEDS_ON(LEDS_RED);
#endif
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(rimac_is_on && radio_is_on != 0 && is_listening == 0 &&
     is_streaming == 0) {
    radio_is_on = 0;
#if DUAL_RADIO
		dual_radio_turn_off(BOTH_RADIO);
#else
    NETSTACK_RADIO.off();
    LEDS_OFF(LEDS_RED);
#endif
  }
}
/*---------------------------------------------------------------------------*/
#if DUAL_RADIO
static void
powercycle_dual_turn_radio_on(char target)
{
  if(we_are_sending == 0 &&
     waiting_for_packet == 0) {
	  dual_radio_on(target);
  }
#if RIMAC_CONF_COMPOWER
  compower_accumulate(&compower_idle_activity);
#endif /* RIMAC_CONF_COMPOWER */
}
static void
powercycle_dual_turn_radio_off(char target)
{
  if(we_are_sending == 0 &&
     waiting_for_packet == 0) {
	  dual_radio_off(target);
  }
}
#else
static void
powercycle_turn_radio_off(void)
{
  if(we_are_sending == 0 &&
     waiting_for_packet == 0) {
    off();
  }
#if RIMAC_CONF_COMPOWER
  compower_accumulate(&compower_idle_activity);
#endif /* RIMAC_CONF_COMPOWER */
}
static void
powercycle_turn_radio_on(void)
{
  if(we_are_sending == 0 &&
     waiting_for_packet == 0) {
    on();
  }
}
#endif
PROCESS_THREAD(strobe_wait, ev, data)
{
	static struct etimer et;
	rtimer_clock_t t;
//	printf("before process begin\n");
	PROCESS_BEGIN();
	if(!is_short_waiting)
	{
		uint8_t *cnt = (uint8_t *)data;
		t = (rimac_config.strobe_time) - (*cnt + 3)*(rimac_config.strobe_wait_time + 1);
		t >= rimac_config.strobe_time ? t = 1 : t;
#if DUAL_RADIO
		dual_radio_off(BOTH_RADIO);
#else
		off();
#endif
	}
	else if (is_short_waiting == 1)
	{
		t = SHORT_SLOT_LEN;
	}
#ifdef ZOUL_MOTE
	else if (is_short_waiting == 2)
	{
		dual_radio_off(BOTH_RADIO);
//		dual_radio_on(SHORT_RADIO);
		t = BEFORE_SHORT_SLOT;
	}
#endif
//	printf("before timer set\n");
	clock_time_t t_wait = (1ul * CLOCK_SECOND * (t)) / RTIMER_ARCH_SECOND;
	etimer_set(&et, t_wait);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
//	printf("after time expired\n");
	if(!is_short_waiting)
	{
#if DUAL_RADIO
		dual_radio_on(LONG_RADIO);
#else
		on();
#endif
		waiting_for_packet = 1;
	}
#if DUAL_RADIO
	else if (is_short_waiting == 1)
	{
		dual_radio_off(SHORT_RADIO);
		waiting_for_packet = 0;
		is_short_waiting = 0;
		interference = 0;
	}
#ifdef ZOUL_MOTE
	else if (is_short_waiting == 2)
	{
		dual_radio_on(SHORT_RADIO);
		waiting_for_packet = 1;

		t = SHORT_SLOT_LEN;
		clock_time_t t_wait = (1ul * CLOCK_SECOND * (t)) / RTIMER_ARCH_SECOND;
		etimer_set(&et, t_wait);
		PROCESS_WAIT_UNTIL(etimer_expired(&et));
		dual_radio_off(SHORT_RADIO);
		waiting_for_packet = 0;
		is_short_waiting = 0;
	}
#endif /* ZOUL_MOTE */
#endif
	PROCESS_END();
}


/*---------------------------------------------------------------------------*/
#if DUAL_RADIO
static void
dual_radio_on(char target)
{
//	printf("dual_radio_on target %d %d\n",target, radio_is_on);
	if(rimac_is_on && radio_is_on == 0) {
		radio_is_on = 1;
		dual_radio_turn_on(target);
		if(target == LONG_RADIO)
		{
			LEDS_ON(LEDS_GREEN);
		}
		if(target == SHORT_RADIO)
		{
			LEDS_ON(LEDS_RED);
		}
		if(target == BOTH_RADIO)
		{
			LEDS_ON(LEDS_GREEN);
			LEDS_ON(LEDS_RED);
		}
	}
}
static void
dual_radio_off(char target)
{
//	printf("dual_radio_off target %d %d\n",target, radio_is_on);
	if(rimac_is_on && radio_is_on != 0 && is_listening == 0 &&
			is_streaming == 0) {
		radio_is_on = 0;
		dual_radio_turn_off(target);
		if(target == LONG_RADIO)
		{
			LEDS_OFF(LEDS_GREEN);
		}
		if(target == SHORT_RADIO)
		{
			LEDS_OFF(LEDS_RED);
		}
		if(target == BOTH_RADIO)
		{
			LEDS_OFF(LEDS_GREEN);
			LEDS_OFF(LEDS_RED);
		}
	}
}

#endif
/*---------------------------------------------------------------------------*/
static struct ctimer cpowercycle_ctimer;
#define CSCHEDULE_POWERCYCLE(rtime) cschedule_powercycle((1ul * CLOCK_SECOND * (rtime)) / RTIMER_ARCH_SECOND)
static char cpowercycle(void *ptr);
static void
cschedule_powercycle(clock_time_t time)
{

  if(rimac_is_on) {
    if(time == 0) {
      time = 1;
    }
    ctimer_set(&cpowercycle_ctimer, time,
               (void (*)(void *))cpowercycle, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static char
cpowercycle(void *ptr)
{
  if(is_streaming) {
    if(!RTIMER_CLOCK_LT(RTIMER_NOW(), stream_until)) {
      is_streaming = 0;
      linkaddr_copy(&is_streaming_to, &linkaddr_null);
      linkaddr_copy(&is_streaming_to_too, &linkaddr_null);
    }
  }

  PT_BEGIN(&pt);

  while(1) {
	  /*
    if(someone_is_sending > 0) {
      someone_is_sending--;
    }*/

    /* If there were a strobe in the air, turn radio on */
/*#if DUAL_RADIO
#if DUAL_ROUTING_CONVERGE
		// JOOONKI is working on this	
		if(dual_duty_cycle_count <= DUAL_DUTY_RATIO-2)
    {
    	dual_duty_cycle_count++;
			if (short_duty_on == 1){
	      powercycle_dual_turn_radio_on(SHORT_RADIO);
			}
    }
    else
    {
    	dual_duty_cycle_count = 0;
			if (short_duty_on == 1 && long_duty_on == 1) {
	      powercycle_dual_turn_radio_on(BOTH_RADIO);
			}	else if (long_duty_on == 1) {
	      powercycle_dual_turn_radio_on(LONG_RADIO);
			}	else if (short_duty_on == 1 ) {
				powercycle_dual_turn_radio_on(SHORT_RADIO);
			}
    }

#else  DUAL_ROUTING_CONVERGE
#if LSA_MAC
#if LSA_R
#if CONVERGE_MODE == 1
		if (LSA_converge == 1)
#elif CONVERGE_MODE == 2
		if (simple_convergence == 1) 
#endif  CONVERGE_MODE
		{
			if (LSA_lr_child == 1) {
				powercycle_dual_turn_radio_on(LONG_RADIO);
			} else {
				powercycle_dual_turn_radio_on(SHORT_RADIO);
			}
		} else {
			powercycle_dual_turn_radio_on(LONG_RADIO);
		}
#else  LSA_R
		powercycle_dual_turn_radio_on(LONG_RADIO);
#endif  LSA_R
#else  LSA_MAC
    if(dual_duty_cycle_count <= DUAL_DUTY_RATIO-2)
    {
    	dual_duty_cycle_count++;
      powercycle_dual_turn_radio_on(SHORT_RADIO);
    }
    else
    {
    	dual_duty_cycle_count = 0;
      powercycle_dual_turn_radio_on(BOTH_RADIO);
    }
#endif  LSA_MAC
#endif  DUAL_ROUTING_CONVERGE
#else	 DUAL_RADIO
    powercycle_turn_radio_on();
#endif  DUAL_RADIO */


	  if(rimac_is_on == 0) {
		  CSCHEDULE_POWERCYCLE(DEFAULT_ON_TIME);
		  PT_YIELD(&pt);
		  continue;
	  }
//     printf("cpowerycle on\n");
  // Carrier Sensing
	powercycle_dual_turn_radio_on(LONG_RADIO);
    CSCHEDULE_POWERCYCLE(CARRIER_SENSING_TIME);
    PT_YIELD(&pt);
	powercycle_dual_turn_radio_off(LONG_RADIO);
//    printf("after CS\n");
    if(NETSTACK_RADIO.receiving_packet() == 1 || interference == 1) { // Backoff
//    	printf("cs detect\n");
    	interference = 0;
    	backoff = 1;
//        printf("interference backoff\n");
    }
    else if(!waiting_for_packet){ // Tx Preamble packet
    	packetbuf_clear();
    	uint8_t preamble[MAX_STROBE_SIZE];
    	int preamble_len, len;
    	uint8_t got_preamble_ack = 0;
    	uint8_t is_dispatch = 0, is_short_preamble_ack = 0, is_long_preamble_ack = 0;
    	uint8_t is_long_broadcast_preamble_ack = 0;
    	uint8_t cnt;
    	struct rimac_hdr *hdr;

    	dual_radio_switch(LONG_RADIO);
		packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &long_linkaddr_node_addr);
//		printf("sending %d\n",packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
    	rtimer_clock_t t;
    	len = NETSTACK_FRAMER.create();
    	preamble_len = len + sizeof(struct rimac_hdr);
    	if(len < 0 || preamble_len > (int)sizeof(preamble)) {
    		/* Failed to send */
    		PRINTF("rimac: send failed, too large header\n");
    		return MAC_TX_ERR_FATAL;
    	}

    	memcpy(preamble, packetbuf_hdrptr(), len);
    	preamble[len] = DISPATCH;
    	preamble[len+1] = TYPE_STROBE;

//    	printf("tx preamble\n");
		if(NETSTACK_RADIO.send(preamble, preamble_len) != RADIO_TX_OK) {
			backoff = 1;
		}
		else {
			//		packetbuf_clear();
			powercycle_dual_turn_radio_on(LONG_RADIO);
			t =  RTIMER_NOW();
			while(got_preamble_ack == 0 &&
					RTIMER_CLOCK_LT(RTIMER_NOW(), t + rimac_config.strobe_wait_time * 2)) {
				if(NETSTACK_RADIO.receiving_packet() == 1) {
					t = RTIMER_NOW();
				}
				packetbuf_clear();
				len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
				if(len > 0) {
					packetbuf_set_datalen(len);
					if(NETSTACK_FRAMER.parse() >= 0) {
						hdr = packetbuf_dataptr();
						char dispatch_ext = hdr->dispatch << 6;
						cnt = hdr->dispatch >> 2;
//						printf("cnt %d\n",cnt);
//						is_dispatch = hdr->dispatch == DISPATCH;
						is_dispatch = dispatch_ext == DISPATCH;
						is_short_preamble_ack = hdr->type == TYPE_STROBE_SHORT_ACK;
						is_long_preamble_ack = hdr->type == TYPE_STROBE_LONG_ACK;
						is_long_broadcast_preamble_ack = hdr->type == TYPE_STROBE_LONG_BROADCAST_ACK;
						if(is_dispatch
								&& (is_short_preamble_ack || is_long_preamble_ack || is_long_broadcast_preamble_ack)) {
							//												  	    	printf("ACK recognized\n");
#if DUAL_RADIO
							if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
									&linkaddr_node_addr) ||
									linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
											&long_linkaddr_node_addr))
#else
								if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
										&linkaddr_node_addr))
#endif
								{
									PRINTF("got preamble_ack %c\n",is_short_preamble_ack ? 'S':'L');
									if(is_short_preamble_ack) {
										got_preamble_ack = SHORT_RADIO;
									}
									else {
										got_preamble_ack = LONG_RADIO;
									}
								}
								else {
									PRINTDEBUG("rimac: strobe ack for someone else\n");
									backoff = 1;
								}
						}
						else {
							PRINTDEBUG("rimac: strobe from someone else\n");
							backoff = 1;
						}
					}
					else {
						PRINTF("rimac: send failed to parse in preamble %u\n", len);
					}
				}
			}
			if(got_preamble_ack) {
				// wait data
				if(got_preamble_ack == SHORT_RADIO) {
					//				printf("sending %d waiting %d\n",we_are_sending,waiting_for_packet);
					powercycle_dual_turn_radio_on(SHORT_RADIO);
					CSCHEDULE_POWERCYCLE(DEFAULT_ON_TIME);
					PT_YIELD(&pt);
				}
				else if(is_long_preamble_ack) {
					powercycle_dual_turn_radio_on(LONG_RADIO);
					CSCHEDULE_POWERCYCLE(DEFAULT_ON_TIME * 2);
					PT_YIELD(&pt);
				}
				else {
			    	  process_start(&strobe_wait, &cnt);
				}
				got_preamble_ack = 0;
			}
		}
    }
/*    powercycle_dual_turn_radio_on(BOTH_RADIO);
    CSCHEDULE_POWERCYCLE(DEFAULT_ON_TIME * 2);
    PT_YIELD(&pt);*/
    if(backoff) { // Do Backoff
//    	printf("backoff\n");
    	backoff = 0;
    	interference = 0;
        powercycle_dual_turn_radio_off(BOTH_RADIO);
        CSCHEDULE_POWERCYCLE(DEFAULT_ON_TIME); // random and exponential backoff
        PT_YIELD(&pt);
    }
    else {
//    	printf("sleep\n");
    	rtimer_clock_t temp;
    	backoff = 0;
    	interference = 0;
    	if(waiting_for_packet != 0) {
    		waiting_for_packet++;
    		if(waiting_for_packet > 2) {
    			waiting_for_packet = 0;
    		}
    	}
    	temp = random_rand()%DEFAULT_OFF_TIME + DEFAULT_OFF_TIME/2;
        powercycle_dual_turn_radio_off(BOTH_RADIO);
        CSCHEDULE_POWERCYCLE(temp);
        PT_YIELD(&pt);
    }

/*    CSCHEDULE_POWERCYCLE(DEFAULT_ON_TIME);
    PT_YIELD(&pt);
    if(rimac_config.off_time > 0) {
#if DUAL_RADIO
      powercycle_dual_turn_radio_off(BOTH_RADIO);
#else
      powercycle_turn_radio_off();
#endif

      if(waiting_for_packet != 0) {
	waiting_for_packet++;
	if(waiting_for_packet > 2) {
	   We should not be awake for more than two consecutive
	     power cycles without having heard a packet, so we turn off
	     the radio.
	  waiting_for_packet = 0;
#if DUAL_RADIO
	  powercycle_dual_turn_radio_off(BOTH_RADIO);
#else
	  powercycle_turn_radio_off();
#endif
	}
      }
      // printf("cpowerycle off\n");
      CSCHEDULE_POWERCYCLE(DEFAULT_OFF_TIME);
      PT_YIELD(&pt);*/
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
#if RIMAC_CONF_ANNOUNCEMENTS
static int
parse_announcements(const linkaddr_t *from)
{
  /* Parse incoming announcements */
  struct announcement_msg adata;
  int i;

  memcpy(&adata, packetbuf_dataptr(), MIN(packetbuf_datalen(), sizeof(adata)));

  /*  printf("%d.%d: probe from %d.%d with %d announcements\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], adata->num);*/
  /*  for(i = 0; i < packetbuf_datalen(); ++i) {
    printf("%02x ", ((uint8_t *)packetbuf_dataptr())[i]);
  }
  printf("\n");*/

  for(i = 0; i < adata.num; ++i) {
    /*   printf("%d.%d: announcement %d: %d\n",
	  linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	  adata->data[i].id,
	  adata->data[i].value);*/

    announcement_heard(from,
		       adata.data[i].id,
		       adata.data[i].value);
  }
  return i;
}
/*---------------------------------------------------------------------------*/
static int
format_announcement(char *hdr)
{
  struct announcement_msg adata;
  struct announcement *a;

  /* Construct the announcements */
  /*  adata = (struct announcement_msg *)hdr;*/

  adata.num = 0;
  for(a = announcement_list();
      a != NULL && adata.num < ANNOUNCEMENT_MAX;
      a = list_item_next(a)) {
    adata.data[adata.num].id = a->id;
    adata.data[adata.num].value = a->value;
    adata.num++;
  }

  memcpy(hdr, &adata, sizeof(struct announcement_msg));

  if(adata.num > 0) {
    return ANNOUNCEMENT_MSG_HEADERLEN +
      sizeof(struct announcement_data) * adata.num;
  } else {
    return 0;
  }
}
#endif /* RIMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
#if WITH_ENCOUNTER_OPTIMIZATION
static void
register_encounter(const linkaddr_t *neighbor, rtimer_clock_t time)
{
  struct encounter *e;

  /* If we have an entry for this neighbor already, we renew it. */
  for(e = list_head(encounter_list); e != NULL; e = list_item_next(e)) {
    if(linkaddr_cmp(neighbor, &e->neighbor)) {
      e->time = time;
      break;
    }
  }
  /* No matching encounter was found, so we allocate a new one. */
  if(e == NULL) {
    e = memb_alloc(&encounter_memb);
    if(e == NULL) {
      /* We could not allocate memory for this encounter, so we just drop it. */
      return;
    }
    linkaddr_copy(&e->neighbor, neighbor);
    e->time = time;
    list_add(encounter_list, e);
  }
}
#endif /* WITH_ENCOUNTER_OPTIMIZATION */
/*---------------------------------------------------------------------------*/
static int
send_packet(void)
{
  rtimer_clock_t t0;
  rtimer_clock_t t;
  rtimer_clock_t encounter_time = 0;
  int strobes;
  struct rimac_hdr *hdr;
  int got_strobe = 0;
#if DATA_ACK
  uint8_t got_data_ack = 0;
#endif
  uint8_t preamble_ack[MAX_STROBE_SIZE];
  int ack_len, len;
  int is_broadcast = 0;
  int is_dispatch, is_strobe;
  /*int is_reliable;*/
  struct encounter *e;
  struct queuebuf *packet;
  int is_already_streaming = 0;
  uint8_t collisions;

  linkaddr_t recv_addr;
  linkaddr_t recv_addr_2;
  linkaddr_t temp_addr;

	/* for debug */
#if TIMING
  static rtimer_clock_t mark_time=0;
#endif
#if PS_COUNT
  rdc_transmission_count++;
#endif
#if DUAL_RADIO
  char target = SHORT_RADIO;
  rtimer_clock_t strobe_time;
#endif
	// JJH
#if DUAL_RADIO
#if LSA_MAC
	static uint8_t was_short;
#endif
#endif

  /* Create the X-MAC header for the data packet. */
#if !NETSTACK_CONF_BRIDGE_MODE
  /* If NETSTACK_CONF_BRIDGE_MODE is set, assume PACKETBUF_ADDR_SENDER is already set. */
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#endif

	/* JOONKI */
#if DUAL_RADIO
	if(sending_in_LR() == LONG_RADIO){
		target = LONG_RADIO;
		strobe_time = rimac_config.strobe_time * DUAL_DUTY_RATIO;
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &long_linkaddr_node_addr);
	}	else	{
		target = SHORT_RADIO;
		strobe_time = rimac_config.strobe_time;
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	}
#else
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#endif
  if(packetbuf_holds_broadcast()) {
    is_broadcast = 1;
    PRINTDEBUG("rimac: send broadcast\n");
  } else {
#if NETSTACK_CONF_WITH_IPV6
    PRINTDEBUG("rimac: send unicast to %02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[2],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[3],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[4],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[5],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[6],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[7]);
#else
    PRINTDEBUG("rimac: send unicast to %u.%u\n",
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1]);
#endif /* NETSTACK_CONF_WITH_IPV6 */
  }
  linkaddr_copy(&recv_addr,packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  if(!is_broadcast) {
	  linkaddr_copy(&recv_addr_2,&recv_addr);
	  if(recv_addr.u8[0]==0x80) {
		  recv_addr_2.u8[0] = 0x00;
	  }
	  else {
		  recv_addr_2.u8[0] = 0x80;
	  }
  }

  len = NETSTACK_FRAMER.create();
  if(len < 0 ||len > (int)sizeof(preamble_ack)) {
	  PRINTF("rimac: send failed, too large header\n");
  }
  packetbuf_compact();
  packet = queuebuf_new_from_packetbuf();
  if(packet == NULL) {
    /* No buffer available */
    PRINTF("rimac: send failed, no queue buffer available (of %u)\n",
           QUEUEBUF_CONF_NUM);
    return MAC_TX_ERR;
  }

#if DUAL_RADIO
  dual_radio_off(BOTH_RADIO);
#else
  off();
#endif


  /* By setting we_are_sending to one, we ensure that the rtimer
     powercycle interrupt do not interfere with us sending the packet. */
  we_are_sending = 1;
  
  t0 = RTIMER_NOW();
  strobes = 0;

 //  LEDS_ON(LEDS_BLUE);

  /* Send a train of strobes until the receiver answers with an ACK. */
	/* Always use long preamble in LSA_MAC mode */
#if DUAL_RADIO
#if LSA_MAC
#if LSA_R
	if (is_broadcast || LSA_SR_preamble == 0) 
	{
		if (sending_in_LR() == SHORT_RADIO){
			was_short = 1;
			dual_radio_switch(LONG_RADIO);
			target = LONG_RADIO;
		}	else	{
			was_short = 0;
		}
	}
#else
	if (sending_in_LR() == SHORT_RADIO){
		was_short = 1;
		dual_radio_switch(LONG_RADIO);
		target = LONG_RADIO;
	}	else	{
		was_short = 0;
	}
#endif /* LSA_R */
#endif /* LSA_MAC */
#endif

	  /* Turn on the radio to listen for the strobe ACK. */
#if COOJA
	if (!((is_broadcast && was_short == 1) || recv_addr.u8[1] == SERVER_NODE)) {
#else
	if (!((is_broadcast && was_short == 1) || recv_addr.u8[7] == SERVER_NODE)) {
#endif
#if DUAL_RADIO
		dual_radio_on(LONG_RADIO);
#else
		on();
#endif
	}
  collisions = 0;
  if(!is_already_streaming) {
	  
#ifndef ZOUL_MOTE
		watchdog_stop();
#else
		watchdog_periodic();
#endif
	  got_strobe = 0;
	  t = RTIMER_NOW();
		
		for(strobes = 0, collisions = 0;
			  got_strobe != 1 && collisions == 0 &&
#if DUAL_RADIO
					  RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + strobe_time);
#else
					  RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + rimac_config.strobe_time);
#endif
			  strobes++) {
#if ZOUL_MOTE
			watchdog_periodic();
#endif
			/* for debug */
#if DUAL_RADIO
#if LSA_MAC
#if COOJA
			if ((is_broadcast && was_short == 1) || recv_addr.u8[1] == SERVER_NODE)
#else
			if ((is_broadcast && was_short == 1) || recv_addr.u8[7] == SERVER_NODE)
#endif
			{
				got_strobe = 1;
				break;
			}
#endif /* LSA_MAC */
#endif

				/* Strobe wait start time fixed */
			while(got_strobe != 1 &&
					RTIMER_CLOCK_LT(RTIMER_NOW(), t + rimac_config.strobe_wait_time)) {
								rtimer_clock_t now = RTIMER_NOW();
				/* See if we got an ACK */
				packetbuf_clear();
				len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
				if(len > 0) {
					packetbuf_set_datalen(len);
					if(NETSTACK_FRAMER.parse() >= 0) {
						hdr = packetbuf_dataptr();
						
						is_dispatch = hdr->dispatch == DISPATCH;
						is_strobe = hdr->type == TYPE_STROBE;
						if(is_dispatch && is_strobe) {
#if DUAL_RADIO

							if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
									&recv_addr) ||
									linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
											&recv_addr_2) ||
											is_broadcast)// ||
/*									linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
											&long_linkaddr_node_addr)) */
#else
								if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
										&linkaddr_node_addr)) 
#endif
								{
//									  printf("here %d\n",packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
									/* We got an ACK from the receiver, so we can immediately send
		   the packet. */
									if(is_broadcast) {
										PRINTF("got strobe but broadcast\n");
										got_strobe = 2;
										break;
									}
									else {
										PRINTF("got strobe\n");
										got_strobe = 1;
										break;
									}
//									encounter_time = now;
								} else {
									PRINTDEBUG("rimac: strobe for someone else\n");
								}
							} else /*if(hdr->dispatch == DISPATCH && hdr->type == TYPE_STROBE)*/ {
								PRINTDEBUG("rimac: not strobe\n");
							}
						} else {
							PRINTF("rimac: send failed to parse send_packet %u\n", len);
						}
					}
				}
				t = RTIMER_NOW();

#if COOJA
	if (!((is_broadcast && was_short == 1) || recv_addr.u8[1] == SERVER_NODE) && got_strobe != 0)
#else
	if (!((is_broadcast && was_short == 1) || recv_addr.u8[7] == SERVER_NODE) && got_strobe != 0)
#endif
	{
//		printf("got_strobe %d\n",got_strobe);
		if(got_strobe == 2) {
			got_strobe = 0;
		}
//	  printf("sender %d\n",packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
	  linkaddr_copy(&temp_addr,packetbuf_addr(PACKETBUF_ADDR_SENDER));
	  packetbuf_clear();
	  packetbuf_set_addr(PACKETBUF_ADDR_SENDER,&long_linkaddr_node_addr);
	  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,&temp_addr);

	  len = NETSTACK_FRAMER.create();
	  ack_len = len + sizeof(struct rimac_hdr);
	  if(len < 0 || ack_len > (int)sizeof(preamble_ack)) {
		  PRINTF("rimac: send failed, too large header\n");
		  return MAC_TX_ERR_FATAL;
	  }
	  memcpy(preamble_ack, packetbuf_hdrptr(), len);
	  preamble_ack[len] = DISPATCH | (++strobes << 2);
	  if(was_short) {
		  preamble_ack[len + 1] = TYPE_STROBE_SHORT_ACK;
	  }
	  else if(is_broadcast){
		  preamble_ack[len + 1] = TYPE_STROBE_LONG_BROADCAST_ACK;
	  }
	  else {
		  preamble_ack[len + 1] = TYPE_STROBE_LONG_ACK;
	  }
	  NETSTACK_RADIO.send(preamble_ack, ack_len);
  }
		}
	  }

#if DUAL_RADIO
  dual_radio_off(target);
#else
  off();
#endif

	/* Switch the radio back to the original one */
/*
#if DUAL_RADIO
#if LSA_MAC
#if LSA_R
	if (is_broadcast || LSA_SR_preamble == 0) {  
		if (was_short == 1)	{
 				dual_radio_switch(SHORT_RADIO);
				target = SHORT_RADIO;
			}
	}
#else
			if (was_short == 1)	{
 				dual_radio_switch(SHORT_RADIO);
				target = SHORT_RADIO;
			}
#endif
#endif
#endif
*/

  /* restore the packet to send */
  queuebuf_to_packetbuf(packet);
  queuebuf_free(packet);

  if(was_short) {
	  target = SHORT_RADIO;
	  dual_radio_switch(SHORT_RADIO);
  }
  else {
	  target = LONG_RADIO;
	  dual_radio_switch(LONG_RADIO);
  }

  /* Send the data packet. */
  if((is_broadcast || got_strobe || is_streaming) && collisions == 0) {
/*		if(is_broadcast && was_short == 1) {
			printf("now2 %d\n",clock_time());
		}
		else
		{
			printf("now %d\n",clock_time());
		}*/
	  NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
	
#if DATA_ACK
		if(!is_broadcast)
		{
			packetbuf_compact();
			packet = queuebuf_new_from_packetbuf();
#if DUAL_RADIO
			dual_radio_on(target);
#else
			on();
#endif
			t = RTIMER_NOW();
			while(got_data_ack == 0 &&
					RTIMER_CLOCK_LT(RTIMER_NOW(), t + rimac_config.strobe_wait_time * 2)) {
				 // printf("wait for data ack %d\n",got_data_ack);
				packetbuf_clear();
				len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
				if(len > 0) {
					// printf("YEEEEEEEEEEAAAAAAAAAAAAAAHHHHHHHHHHHHHHH????\n");
					packetbuf_set_datalen(len);
					if(NETSTACK_FRAMER.parse() >= 0) {
						hdr = packetbuf_dataptr();
						// printf("after parsing type %x\n",hdr->type);
						if(hdr->type == TYPE_DATA_ACK) {
#if DUAL_RADIO
							if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
									&linkaddr_node_addr) ||
									linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
											&long_linkaddr_node_addr))
#else
								if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
										&linkaddr_node_addr))
#endif
								{
									got_data_ack = 1;
									//PRINTDEBUG("rimac: got data ack\n");
								} else {
									PRINTDEBUG("rimac: data ack for someone else\n");
								}
						}
					} else {
						PRINTF("rimac: send failed to parse %u\n", len);
					}
				}
			}
			queuebuf_to_packetbuf(packet);
			queuebuf_free(packet);
#if DUAL_RADIO
			dual_radio_off(target);
#else
			off();
#endif
		}
#endif

  }

#if WITH_ENCOUNTER_OPTIMIZATION
  if(got_strobe && !is_streaming) {
    register_encounter(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), encounter_time);
  }
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

#ifndef ZOUL_MOTE
  watchdog_start();
#endif
	/* For debug */
  PRINTF("rimac: send (strobes=%u,len=%u,%s), done\n", strobes,
	 packetbuf_totlen(), got_strobe ? "ack" : "no ack");
#if DATA_ACK
  if(!is_broadcast && got_strobe)
  {
	  PRINTF("rimac: recv %s\n",got_data_ack ? "data_ack" : "data_noack");
  }

#endif
#if RIMAC_CONF_COMPOWER
  /* Accumulate the power consumption for the packet transmission. */
  compower_accumulate(&current_packet);

  /* Convert the accumulated power consumption for the transmitted
     packet to packet attributes so that the higher levels can keep
     track of the amount of energy spent on transmitting the
     packet. */
  compower_attrconv(&current_packet);

  /* Clear the accumulated power consumption so that it is ready for
     the next packet. */
  compower_clear(&current_packet);
#endif /* RIMAC_CONF_COMPOWER */

  we_are_sending = 0;

  LEDS_OFF(LEDS_BLUE);
  dual_radio_off(BOTH_RADIO);

  if(collisions == 0) {
#if DATA_ACK
//	  printf("rimac status %d %d %d\n",is_broadcast,got_strobe,got_data_ack);
    if(!is_broadcast && (!got_strobe || !got_data_ack)) 
#else
    if(!is_broadcast && !got_strobe) 
#endif
		{
      return MAC_TX_NOACK;
    } else {
      return MAC_TX_OK;
    }
  } else {
    someone_is_sending++;
    return MAC_TX_COLLISION;
  }
}
/*---------------------------------------------------------------------------*/
static void
qsend_packet(mac_callback_t sent, void *ptr)
{
  int ret;
  if(someone_is_sending) {
    PRINTF("rimac: should queue packet, now just dropping %d %d %d %d.\n",
	   waiting_for_packet, someone_is_sending, we_are_sending, radio_is_on);
    RIMESTATS_ADD(sendingdrop);
    ret = MAC_TX_COLLISION;
  } else {
    PRINTF("rimac: send immediately.\n");
    ret = send_packet();
  }
  mac_call_sent_callback(sent, ptr, ret, 1);
}
/*---------------------------------------------------------------------------*/
static void
qsend_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  if(buf_list != NULL) {
    queuebuf_to_packetbuf(buf_list->buf);
    qsend_packet(sent, ptr);
		/* Switch the radio back to the original one */
  }
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
	interference = 1;
	struct rimac_hdr *hdr;
	// JJH
#if DUAL_RADIO
  int target = SHORT_RADIO;
#endif
#if RPL_LIFETIME_MAX_MODE2
  int original_datalen;
  uint8_t *original_dataptr;
#endif
#if LSA_MAC
	uint8_t for_short = 1;
#endif 
#if DATA_ACK
	struct queuebuf *packet;
#endif

  if(NETSTACK_FRAMER.parse() >= 0) {
    hdr = packetbuf_dataptr();
    char dispatch_ext = hdr->dispatch << 6;
//	PRINTF("why here? %d %x\n",
//			hdr->dispatch,hdr->type);

#if RPL_LIFETIME_MAX_MODE2
    original_datalen = packetbuf_totlen();
    original_dataptr = packetbuf_dataptr();
#endif
/*
#if STROBE_CNT_MODE
    char dispatch_ext = hdr->dispatch << 6;
//    printf("packet input dispatch %d\n",dispatch_ext);
*/
//#if DATA_ACK
//    if(dispatch_ext != DISPATCH
//    		|| (hdr->type != TYPE_STROBE_ACK && hdr->type != TYPE_STROBE && hdr->type != TYPE_ANNOUNCEMENT && hdr->type != TYPE_DATA_ACK))
/*
#else
    if(dispatch_ext != DISPATCH
       		|| (hdr->type != TYPE_STROBE_ACK && hdr->type != TYPE_STROBE && hdr->type != TYPE_ANNOUNCEMENT))
*/

//#endif
//#else
//    if(hdr->dispatch != DISPATCH)
#if DATA_ACK
    if(dispatch_ext != DISPATCH
    		|| (hdr->type != TYPE_STROBE && hdr->type != TYPE_STROBE_SHORT_ACK && hdr->type != TYPE_STROBE_LONG_ACK
    				&& hdr->type != TYPE_STROBE_LONG_BROADCAST_ACK && hdr->type != TYPE_DATA_ACK))
#else
    if(dispatch_ext != DISPATCH
       		|| (hdr->type != TYPE_STROBE && hdr->type != TYPE_STROBE_SHORT_ACK && hdr->type != TYPE_STROBE_LONG_ACK
       				&& hdr->type != TYPE_STROBE_LONG_BROADCAST_ACK))

#endif
//#endif
		{		// The packet is for data
      someone_is_sending = 0;
#if DUAL_RADIO
      if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                           &linkaddr_node_addr) ||
      	 linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
      			 	 	 	 	 	 	   &linkaddr_null) ||
		linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
		     					     &long_linkaddr_node_addr)) 
#else
      if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                     &linkaddr_node_addr) ||
	 linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                      &linkaddr_null)) 
	/* This is a regular packet that is destined to us or to the
	   broadcast address. */

	/* We have received the final packet, so we can go back to being
	   asleep. */
#endif
				{


#if DUAL_RADIO
#if LSA_MAC
//				 JOONKI
//				 waiting for incoming short broadcast
				if (linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_null) && 
						radio_received_is_longrange()==LONG_RADIO){
#ifdef	COOJA
//					printf("long broadcast\n");
					dual_radio_off(LONG_RADIO);
					dual_radio_on(SHORT_RADIO);
					waiting_for_packet = 1;
					is_short_waiting = 1;
					process_start(&strobe_wait, NULL);
#endif
#if ZOUL_MOTE
//					is_short_waiting=2;
					dual_radio_off(LONG_RADIO);
					dual_radio_on(SHORT_RADIO);
					waiting_for_packet = 1;
					is_short_waiting = 1;
					process_start(&strobe_wait,NULL);
#endif
				}	else{
					dual_radio_off(BOTH_RADIO);
					waiting_for_packet = 0;
				}
#else // LSA_MAC
				dual_radio_off(BOTH_RADIO);
				waiting_for_packet = 0;
#endif // LSA_MAC
#else
    	  off();
    	  waiting_for_packet = 0;
#endif


#if RIMAC_CONF_COMPOWER
	/* Accumulate the power consumption for the packet reception. */
	compower_accumulate(&current_packet);
	/* Convert the accumulated power consumption for the received
	   packet to packet attributes so that the higher levels can
	   keep track of the amount of energy spent on receiving the
	   packet. */
	compower_attrconv(&current_packet);

	/* Clear the accumulated power consumption so that it is ready
	   for the next packet. */
	compower_clear(&current_packet);
#endif /* RIMAC_CONF_COMPOWER */

//	printf("rx data\n");
#if RPL_LIFETIME_MAX_MODE2
	if(!linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &linkaddr_null))
	{
/*	printf("data point %c %c %c %c %c %c %c %c len:%d\n\n",original_dataptr[original_datalen-1],
			original_dataptr[original_datalen-11],original_dataptr[original_datalen-21],
			original_dataptr[original_datalen-31],original_dataptr[original_datalen-41],
			original_dataptr[original_datalen-51],original_dataptr[original_datalen-61],
			original_dataptr[original_datalen-50],original_datalen);*/
#if COOJA
	if(original_dataptr[original_datalen-50]=='X' && packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1] != SERVER_NODE) // Data packet inspection app start -50
	{
		int recv_id = 0;
		int ip = 0;
//	    recv_id = (original_dataptr[original_datalen-32] - '0') + (original_dataptr[original_datalen-33] - '0')*10
//	    		+ (original_dataptr[original_datalen-34]- '0')*100 + (original_dataptr[original_datalen-35]- '0')*1000;
	    recv_id = (original_dataptr[original_datalen-60] - '0') + (original_dataptr[original_datalen-61] - '0')*10
	    		+ (original_dataptr[original_datalen-62]- '0')*100 + (original_dataptr[original_datalen-63]- '0')*1000;
/*	    ip = (original_dataptr[original_datalen-23] - '0') + (original_dataptr[original_datalen-24] - '0')*10
	    		+ (original_dataptr[original_datalen-25]- '0')*100;*/
	    ip = (original_dataptr[original_datalen-51] - '0') + (original_dataptr[original_datalen-52] - '0')*10
	    		+ (original_dataptr[original_datalen-53]- '0')*100;
//	    printf("incoming recv_id %d ip %d\n",recv_id, ip);
#else
	 if(original_dataptr[original_datalen-50]=='X' && packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[7] != SERVER_NODE) // Data packet inspection app start -50
	 {
		 int recv_id = 0;
		 int ip = 0;
		 //	    recv_id = (original_dataptr[original_datalen-32] - '0') + (original_dataptr[original_datalen-33] - '0')*10
		 //	    		+ (original_dataptr[original_datalen-34]- '0')*100 + (original_dataptr[original_datalen-35]- '0')*1000;
		 recv_id = (original_dataptr[original_datalen-60] - '0') + (original_dataptr[original_datalen-61] - '0')*10
				 + (original_dataptr[original_datalen-62]- '0')*100 + (original_dataptr[original_datalen-63]- '0')*1000;
		 /*	    ip = (original_dataptr[original_datalen-23] - '0') + (original_dataptr[original_datalen-24] - '0')*10
		    		+ (original_dataptr[original_datalen-25]- '0')*100;*/
		 ip = (original_dataptr[original_datalen-51] - '0') + (original_dataptr[original_datalen-52] - '0')*10
				 + (original_dataptr[original_datalen-53]- '0')*100;
//		 printf("incoming recv_id %d ip %d\n",recv_id, ip);
#endif
	    if(id_array[ip] >= recv_id)
	    {
//	    	printf("rimac: duplicated data %d\n",recv_id);
	    }
	    else
	    {
#if DUAL_RADIO
#if ADDR_MAP
//	    	if(long_ip_from_lladdr_map(packetbuf_addr(PACKETBUF_ADDR_SENDER)))

	    	// Update latest recv_id & avg_est_load
/*	    	if(recv_id > latest_id + 1)
	    	{
	    		latest_id = recv_id-1;

	    	}*/
//	    	printf("recv sender ip: %d %d\n",
//	    	packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0],packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[1]);
	    	if(packetbuf_addr(PACKETBUF_ADDR_SENDER)->u8[0] == 0x80)
	    	{
////	    		printf("long!\n");
		    	id_count[recv_id%BUF_SIZE]+=LONG_WEIGHT_RATIO;
	    	}
	    	else
	    	{
//	    		printf("short!\n");
	    		id_count[recv_id%BUF_SIZE]++;
	    	}
#else
	    	id_count[recv_id%BUF_SIZE]++;
#endif
#endif
	    	id_array[ip] = recv_id;
//	    	printf("id: %d count: %d\n",recv_id,id_count[recv_id]);
	    }
	 }
	}
#endif /* RPL_ENERGY_MODE */

#if DATA_ACK
	if(!linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&linkaddr_null)) // Only when it is not broadcast data
	{
		// struct rimac_hdr *hdr;
		uint8_t ack[MAX_STROBE_SIZE];
		uint8_t ack_len, len;
		linkaddr_t temp;
		// Copying original packetbuf
		packet = queuebuf_new_from_packetbuf();
#if DUAL_RADIO
#if LSA_MAC
		if (radio_received_is_longrange()==LONG_RADIO){
			dual_radio_switch(LONG_RADIO);
		}	else if (radio_received_is_longrange() == SHORT_RADIO){
			dual_radio_switch(SHORT_RADIO);
		}
#endif
#endif
		/* JOONKI
		 * Not sure why this is working */
#ifdef ZOUL_MOTE
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,
			   packetbuf_addr(PACKETBUF_ADDR_SENDER));
#else
		linkaddr_copy(&temp,packetbuf_addr(PACKETBUF_ADDR_SENDER));
		packetbuf_clear();
		packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,&temp);
#endif

#if DUAL_RADIO
		if(sending_in_LR() == LONG_RADIO)
		{
			packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &long_linkaddr_node_addr);
		}
		else
		{
			packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
		}
#else
		packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#endif
		
		len = NETSTACK_FRAMER.create();
		if(len < 0)
		{
			PRINTF("rimac: failed to send data ack\n");
			return;
		}
		ack_len = len + sizeof(struct rimac_hdr);
		memcpy(ack,packetbuf_hdrptr(),len);
		ack[len] = DISPATCH;
		ack[len + 1] = TYPE_DATA_ACK;
		
		// hdr->type = TYPE_DATA_ACK;

		
		/* rtimer_clock_t wait;
		wait=RTIMER_NOW();
		while(RTIMER_CLOCK_LT(RTIMER_NOW(), wait + rimac_config.strobe_wait_time)); */
		NETSTACK_RADIO.send(ack, ack_len);
		
		// is_short_waiting = 1;
		// process_start(&strobe_wait, NULL);
	  // NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
		// LOG_MESSAGE("SEDNING DATA ACK!!!!!! %d\n",NETSTACK_RADIO.send(ack, ack_len));
		// printf("rimac: send data ack %u\n", ack_len);
		// printf("rimac: send data ack %u\n", packetbuf_totlen());
	}
	queuebuf_to_packetbuf(packet);
	queuebuf_free(packet);
#endif

        PRINTDEBUG("rimac: data(%u)\n", packetbuf_datalen());
        NETSTACK_MAC.input();
        if(!(waiting_for_packet && is_short_waiting)) {
        	dual_radio_off(BOTH_RADIO);
        }
        return;
      } else {
        PRINTDEBUG("rimac: data not for us\n");
      }

    } else if(hdr->type == TYPE_STROBE) {
 /*     someone_is_sending = 2;
#if DUAL_RADIO
				if (radio_received_is_longrange()==LONG_RADIO){
					dual_radio_switch(LONG_RADIO);
				}	else if (radio_received_is_longrange() == SHORT_RADIO){
					dual_radio_switch(SHORT_RADIO);
				}
#endif
#if DUAL_RADIO
      if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                      &linkaddr_node_addr) ||
    	 linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
    		  		     &long_linkaddr_node_addr)) 
#else
      if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                      &linkaddr_node_addr)) 
#endif
			{	
#if DUAL_RADIO
#if LSA_MAC
				if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&linkaddr_node_addr) == 1){
					for_short = 1;
				} else if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),&long_linkaddr_node_addr) == 1) {
					for_short = 0;
				}
#endif
#endif
	 This is a strobe packet for us.

	 If the sender address is someone else, we should
	   acknowledge the strobe and wait for the packet. By using
	   the same address as both sender and receiver, we flag the
	   message is a strobe ack.
	hdr->type = TYPE_STROBE_ACK;
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,
			   packetbuf_addr(PACKETBUF_ADDR_SENDER));
 
#if DUAL_RADIO
	if(sending_in_LR() == LONG_RADIO){
		target = LONG_RADIO;
		packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &long_linkaddr_node_addr);
	}	else	{
		target = SHORT_RADIO;
		packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
	}
#else
  	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
#endif
	packetbuf_compact();
	if(NETSTACK_FRAMER.create() >= 0) {
	   We turn on the radio in anticipation of the incoming
	     packet.
	  someone_is_sending = 1;
	  waiting_for_packet = 1;

#if DUAL_RADIO
#if LSA_MAC
		dual_radio_off(BOTH_RADIO);
		if (for_short == 1) {
			target = SHORT_RADIO;
		} else if (for_short == 0) {
			target = LONG_RADIO;
		}
#endif
#endif

#if DUAL_RADIO
	  dual_radio_on(target);
#else
	  on();
#endif
	  // LOG_MESSAGE("SENDING STROBE ACK %d\n",NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen()));
	  NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
	 // printf("rimac: send strobe ack %u\n", packetbuf_totlen());
	} else {
	  PRINTF("rimac: failed to send strobe ack\n");
	}
      } else if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                             &linkaddr_null)) {
	 If the receiver address is null, the strobe is sent to
	   prepare for an incoming broadcast packet. If this is the
	   case, we turn on the radio and wait for the incoming
	   broadcast packet.
//    	  printf("rimac strobe_cnt %d\n",hdr->dispatch >> 2);
    	  waiting_for_packet = 1;
#if STROBE_CNT_MODE
    	  uint8_t cnt = hdr->dispatch >> 2;
#if DUAL_RADIO
    	  strobe_target = radio_received_is_longrange();
    	  dual_radio_off(BOTH_RADIO);
    	   Wait based on strobe count, to Rx data
    	  process_start(&strobe_wait, &cnt);
#else
    	  off();
    	  process_start(&strobe_wait, &cnt);
#endif
#endif	 STROBE_CNT_MODE


      } else {
        PRINTDEBUG("rimac: strobe not for us\n");
      }*/

      /* We are done processing the strobe and we therefore return
	 to the caller. */
//        PRINTDEBUG("rimac: stray strobe\n");
        return;
#if RIMAC_CONF_ANNOUNCEMENTS
    } else if(hdr->type == TYPE_ANNOUNCEMENT) {
      packetbuf_hdrreduce(sizeof(struct rimac_hdr));
      parse_announcements(packetbuf_addr(PACKETBUF_ADDR_SENDER));
#endif /* RIMAC_CONF_ANNOUNCEMENTS */
    } else if(hdr->type == TYPE_STROBE_SHORT_ACK || hdr->type == TYPE_STROBE_LONG_ACK) {
//      PRINTDEBUG("rimac: stray strobe ack\n");
    }
#if DATA_ACK
    else if(hdr->type == TYPE_DATA_ACK) {
    	PRINTDEBUG("rimac: stray data_ack\n");
    }
#endif
    else {
      PRINTF("rimac: unknown type %u (%u)\n", hdr->type,
             packetbuf_datalen());
    }
  } else {
    PRINTF("rimac: failed to parse (%u)\n", packetbuf_totlen());
  }
}
/*---------------------------------------------------------------------------*/
#if RIMAC_CONF_ANNOUNCEMENTS
static void
send_announcement(void *ptr)
{
  struct rimac_hdr *hdr;
  int announcement_len;

  /* Set up the probe header. */
  packetbuf_clear();
  hdr = packetbuf_dataptr();

  announcement_len = format_announcement((char *)hdr +
					 sizeof(struct rimac_hdr));

  if(announcement_len > 0) {
    packetbuf_set_datalen(sizeof(struct rimac_hdr) + announcement_len);
    hdr->dispatch = DISPATCH;
    hdr->type = TYPE_ANNOUNCEMENT;

    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
    packetbuf_set_attr(PACKETBUF_ATTR_RADIO_TXPOWER, announcement_radio_txpower);
    if(NETSTACK_FRAMER.create() >= 0) {
      NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
cycle_announcement(void *ptr)
{
  ctimer_set(&announcement_ctimer, ANNOUNCEMENT_TIME,
	     send_announcement, NULL);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_PERIOD,
	     cycle_announcement, NULL);
  if(is_listening > 0) {
    is_listening--;
    /*    printf("is_listening %d\n", is_listening);*/
  }
}
/*---------------------------------------------------------------------------*/
static void
listen_callback(int periods)
{
  is_listening = periods + 1;
}
#endif /* RIMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
void
rimac_set_announcement_radio_txpower(int txpower)
{
#if RIMAC_CONF_ANNOUNCEMENTS
  announcement_radio_txpower = txpower;
#endif /* RIMAC_CONF_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
void
rimac_init(void)
{
#if TIMING
	printf("rimac on_time %d\n",DEFAULT_ON_TIME*10000/RTIMER_ARCH_SECOND);
	printf("rimac off_time %d\n",DEFAULT_OFF_TIME*10000/RTIMER_ARCH_SECOND);
	printf("rimac strobe_wait_time %d\n",rimac_config.strobe_wait_time*10000/RTIMER_ARCH_SECOND);
	printf("rimac strobe %d\n",rimac_config.strobe_time*10000/RTIMER_ARCH_SECOND);
	printf("------------------------------------------\n");
#endif
#if DUAL_RADIO
  dual_duty_cycle_count = 0;
#if DUAL_ROUTING_CONVERGE
	long_duty_on = 1;
	short_duty_on = 1;
#endif
#if LSA_R
	LSA_converge = 0;
	LSA_SR_preamble = 0;
	LSA_message_input = 0;
	LSA_broadcast_count = 1;
#endif

#endif
#if RPL_LIFETIME_MAX_MODE2
	MLS = 0; // Initialize MLS
#endif
  radio_is_on = 0;
  waiting_for_packet = 0;
  interference = 0;
  backoff = 0;
  PT_INIT(&pt);
  /*  rtimer_set(&rt, RTIMER_NOW() + rimac_config.off_time, 1,
      (void (*)(struct rtimer *, void *))powercycle, NULL);*/

  rimac_is_on = 1;

#if WITH_ENCOUNTER_OPTIMIZATION
  list_init(encounter_list);
  memb_init(&encounter_memb);
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

#if RIMAC_CONF_ANNOUNCEMENTS
  announcement_register_listen_callback(listen_callback);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_TIME,
	     cycle_announcement, NULL);
#endif /* RIMAC_CONF_ANNOUNCEMENTS */

  CSCHEDULE_POWERCYCLE(DEFAULT_OFF_TIME);
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  rimac_is_on = 1;
  /*  rtimer_set(&rt, RTIMER_NOW() + rimac_config.off_time, 1,
      (void (*)(struct rtimer *, void *))powercycle, NULL);*/
  CSCHEDULE_POWERCYCLE(DEFAULT_OFF_TIME);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  rimac_is_on = 0;
#if DUAL_RADIO
  if(keep_radio_on) {
    return dual_radio_turn_on(BOTH_RADIO);
  } else {
    return dual_radio_turn_off(BOTH_RADIO);
  }
#else
  if(keep_radio_on) {
    return NETSTACK_RADIO.on();
  } else {
    return NETSTACK_RADIO.off();
  }
#endif
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return (1ul * CLOCK_SECOND * DEFAULT_PERIOD) / RTIMER_ARCH_SECOND;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver rimac_driver =
  {
    "RI-MAC",
    rimac_init,
    qsend_packet,
    qsend_list,
    input_packet,
    turn_on,
    turn_off,
    channel_check_interval,
  };
