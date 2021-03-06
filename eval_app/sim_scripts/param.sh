#!/bin/bash
APP=${11}

if [ $APP -eq 0 ]
then
    PATTH="../../project-conf.h"
else
    PATTH="../../../lanada_$APP/project-conf.h"
fi
echo "#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Energy log */
#define RPL_ICMP_ENERGY_LOG		0

uint8_t dead;

/* RPL Configuration */

#define RPL_CONF_LONG_WEIGHT_RATIO	$4
#define RPL_CONF_ETX_WEIGHT	$5
#define RPL_CONF_BETA	$6
#define RPL_CONF_BETA_DIV	$7
#define RPL_CONF_CROSS_OPT_VERSION1	$8
#define RPL_CONF_DUAL_RPL_RECAL_MODE ${12}
#define RPL_CONF_DUAL_RPL_PROB_PARENT_SWITCH 1

/* MAC Configuration */
#define MAC_CONF_STROBE_CNT_MODE	$9
#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE ${10}

/* DUAL_RADAIO Configuration */

#define CONF_RESIDUAL_ENERGY_MAX 1000000

/*-----------------------------------------------------------------------------------------------*/
#define DETERMINED_ROUTING_TREE	0

#if DETERMINED_ROUTING_TREE
#define MAX_NODE_NUMBER 30

#endif /* ROUTING_TREE */

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#undef UIP_CONF_MAX_ROUTES

//#ifdef TEST_MORE_ROUTES
/* configure number of neighbors and routes */
#define NBR_TABLE_CONF_MAX_NEIGHBORS     70
//#define UIP_CONF_MAX_ROUTES   30
//#else
/* configure number of neighbors and routes */
//#define NBR_TABLE_CONF_MAX_NEIGHBORS     30
//#define UIP_CONF_MAX_ROUTES   20
//#endif /* TEST_MORE_ROUTES */

#undef NETSTACK_CONF_RDC
// #define NETSTACK_CONF_RDC     nullrdc_driver
// #define NETSTACK_CONF_RDC     contikimac_driver
#define NETSTACK_CONF_RDC     dualmac_driver
// #define NETSTACK_CONF_RDC     rimac_driver

#undef NETSTACK_CONF_MAC
// #define NETSTACK_CONF_MAC     nullmac_driver
#define NETSTACK_CONF_MAC		csma_driver

#define TRAFFIC_MODEL $1 // 0: Periodic, 1: Poisson
#if TRAFFIC_MODEL == 0
#define PERIOD $2
#elif TRAFFIC_MODEL == 1
#define ARRIVAL_RATE $3 // Mean value, 1/lambda
#endif

#undef NULLRDC_CONF_802154_AUTOACK
#define NULLRDC_CONF_802154_AUTOACK       1

/* Define as minutes */
#define RPL_CONF_DEFAULT_LIFETIME_UNIT   60

/* 10 minutes lifetime of routes */
#define RPL_CONF_DEFAULT_LIFETIME        10

#define RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME 1

#undef NULLRDC_CONF_ACK_WAIT_TIME
#define NULLRDC_CONF_ACK_WAIT_TIME	RTIMER_SECOND / 30

#endif /* PROJECT_CONF_H_ */" > $PATH
