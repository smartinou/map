struct device_info
{
    char mFirmwareVersion[32];
    char BuildDate[32];
    char BuildTime[32];
    char GitHash[32];
    char DataBaseStatus[32];
    float Temperature;
};

int gatt_svr_svc_dev_info_init(void);
int gatt_svr_svc_dev_info_set(struct device_info *DevInfo);
