#!/bin/bash
echo "/* Energy log */
#define RPL_ICMP_ENERGY_LOG		0

#define TRAFFIC_MODEL $1 // 0: Periodic, 1: Poisson
#if TRAFFIC_MODEL == 0
#define PERIOD $2
#elif TRAFFIC_MODEL == 1
#define ARRIVAL_RATE $3 // Mean value, 1/lambda
#endif

uint8_t dead;

/* RPL Configuration */
#define RPL_CONF_OF rpl_ltmax_of
// rpl_ltmax_of, rpl_of0

#define RPL_CONF_LONG_WEIGHT_RATIO	$4
#define RPL_CONF_ETX_WEIGHT	$5
#define RPL_CONF_BETA	$6
#define RPL_CONF_BETA_DIV	$7
#define RPL_CONF_CROSS_OPT_VERSION1	$8
#define RPL_CONF_DUAL_RPL_RECAL_MODE 1
#define RPL_CONF_DUAL_RPL_PROB_PARENT_SWITCH 0

/* MAC Configuration */
#define MAC_CONF_STROBE_CNT_MODE	$9
#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE ${10}
#define NETSTACK_CONF_MAC csma_driver
#define NETSTACK_CONF_RDC dualmac_driver

/* DUAL_RADAIO Configuration */
#define CONF_DUAL_RADIO ${11}
#define CONF_ONLY_LONG ${12}
#define CONF_WAKEUP_RADIO ${13}

/*-----------------------------------------------------------------------------------------------*/
#define DETERMINED_ROUTING_TREE	0

#if DETERMINED_ROUTING_TREE
#define MAX_NODE_NUMBER 30

#endif /* ROUTING_TREE */" > ../../param.h
