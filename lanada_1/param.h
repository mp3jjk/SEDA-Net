/* Energy log */
#define RPL_ICMP_ENERGY_LOG		0

#define TRAFFIC_MODEL 0 // 0: Periodic, 1: Poisson
#if TRAFFIC_MODEL == 0
#define PERIOD 15
#elif TRAFFIC_MODEL == 1
#define ARRIVAL_RATE 0 // Mean value, 1/lambda
#endif

uint8_t dead;

/* RPL Configuration */

#define RPL_CONF_LONG_WEIGHT_RATIO	1
#define RPL_CONF_ETX_WEIGHT	1
#define RPL_CONF_BETA	1
#define RPL_CONF_BETA_DIV	1
#define RPL_CONF_CROSS_OPT_VERSION1	0
#define RPL_CONF_DUAL_RPL_RECAL_MODE 0
#define RPL_CONF_DUAL_RPL_PROB_PARENT_SWITCH 0

/* MAC Configuration */
#define MAC_CONF_STROBE_CNT_MODE	0
#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8

/* DUAL_RADAIO Configuration */

#define CONF_RESIDUAL_ENERGY_MAX 4000000

/*-----------------------------------------------------------------------------------------------*/
#define DETERMINED_ROUTING_TREE	0

#if DETERMINED_ROUTING_TREE
#define MAX_NODE_NUMBER 30

#endif /* ROUTING_TREE */
