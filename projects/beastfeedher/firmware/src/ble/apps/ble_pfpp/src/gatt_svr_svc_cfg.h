struct time
{
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
};

struct date
{
    uint8_t Year;
    uint8_t Month;
    uint8_t Date;
};

struct feeding
{
    uint8_t Time;
    bool PadEnable;
    bool TimesOfDay[24 * 4];
};

struct cfg
{
    struct time CurrentTime;
    struct date CurrentDate;
    struct feeding Feeding; 
};


//extern struct cfg sCfg;

int gatt_svr_svc_cfg_set(struct cfg *Cfg);
int gatt_svr_svc_cfg_init(void);
