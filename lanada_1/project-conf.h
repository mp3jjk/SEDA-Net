#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#include "param.h"

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#undef UIP_CONF_MAX_ROUTES

//#ifdef TEST_MORE_ROUTES
/* configure number of neighbors and routes */
#define NBR_TABLE_CONF_MAX_NEIGHBORS     50
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

#define TRAFFIC_MODEL 0 // 0: Periodic, 1: Poisson
#if TRAFFIC_MODEL == 0
#define PERIOD 60
#elif TRAFFIC_MODEL == 1
#define ARRIVAL_RATE 0 // Mean value, 1/lambda
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

#endif /* PROJECT_CONF_H_ */
