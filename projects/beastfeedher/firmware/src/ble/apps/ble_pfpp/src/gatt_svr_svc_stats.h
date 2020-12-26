struct network_stats
{
    uint32_t PacketsSent;
    uint32_t PacketsReceived;
    uint32_t PacketsForwarded;
    uint32_t PacketsDropped;
    uint32_t ChecksumErrors;
    uint32_t InvalidLengthErrors;
    uint32_t MemoryErrors;
    uint32_t RoutingErrors;
    uint32_t ProtocolErrors;
    uint32_t OptionErrors;
    uint32_t MiscErrors;
};

struct stats
{
    struct network_stats NetStats;
};


int gatt_svr_svc_stats_set(struct stats *Stats);
int gatt_svr_svc_stats_init(void);
