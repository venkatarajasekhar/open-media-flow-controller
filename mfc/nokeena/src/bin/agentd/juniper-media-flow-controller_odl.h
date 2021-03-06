/* FILE: juniper-media-flow-controller_odl.h -- generated automagikally; DO NOT EDIT */

#ifndef ODL_JUNIPER_MEDIA_FLOW_CONTROLLER_H
#define ODL_JUNIPER_MEDIA_FLOW_CONTROLLER_H


/*
 * Use C linkage when using a C++ compiler
*/
#ifdef __cplusplus
extern "C" {
    namespace junos {
#endif /* __cplusplus */

extern const xml_tag_t odl_juniper_media_flow_controller_tags[];

#define XML_ARRAY odl_juniper_media_flow_controller_tags

#define XML_NS "http://xml.juniper.net/junos/12.1I0/mfc-cluster"

#define ODCI_ACCESSLOG 1
#define ODCI_ACCESSLOG_PROFILE 2
#define ODCI_ACTIVE 3
#define ODCI_ACTUAL_MAX 4
#define ODCI_ADDRESS 5
#define ODCI_ADMIN_STATE 6
#define ODCI_ALIAS 7
#define ODCI_ALIAS_DETAILS 8
#define ODCI_ALIAS_NAME 9
#define ODCI_ALIAS_OF 10
#define ODCI_ARCHITECTURE 11
#define ODCI_ARP 12
#define ODCI_AUTO_GENERATE 13
#define ODCI_AUTO_GENERATE_CFG 14
#define ODCI_BACKUP_FILE 15
#define ODCI_BOND 16
#define ODCI_BOND_DETAIL 17
#define ODCI_BOND_LIST 18
#define ODCI_BOOT_ORDER 19
#define ODCI_BR_IF 20
#define ODCI_BRIDGE 21
#define ODCI_BUS 22
#define ODCI_BYTES 23
#define ODCI_CACHE 24
#define ODCI_CACHE_AGE_DEFAULT 25
#define ODCI_CACHE_HIT 26
#define ODCI_CAPACITY 27
#define ODCI_CFG_ORIGIN 28
#define ODCI_CLIENT_BANDWIDTH 29
#define ODCI_CLIENT_REQUEST 30
#define ODCI_CLIENT_SESSIONS 31
#define ODCI_COMMENT 32
#define ODCI_CONFIG 33
#define ODCI_CONFIG_VERSION 34
#define ODCI_CONFIGURED_MAX 35
#define ODCI_CONNECTION 36
#define ODCI_CONSOLES 37
#define ODCI_COUNT 38
#define ODCI_CPU 39
#define ODCI_CRAWL_FILE_TYPE 40
#define ODCI_CRAWL_STATUS 41
#define ODCI_CRAWLER 42
#define ODCI_CRAWLER_CFG_STATUS 43
#define ODCI_CRAWLER_NAME 44
#define ODCI_DEPTH 45
#define ODCI_DHCP 46
#define ODCI_DISK 47
#define ODCI_DISK_CACHE_STATUS 48
#define ODCI_DISKSPACE 49
#define ODCI_DISTRIBUTION_ID 50
#define ODCI_DOMAIN 51
#define ODCI_DOWN_DEL_TIME 52
#define ODCI_DRIVE 53
#define ODCI_DRIVE_NUM 54
#define ODCI_DUPLEX 55
#define ODCI_ENABLED 56
#define ODCI_ERROR_INFO 57
#define ODCI_ERROR_MESSAGE 58
#define ODCI_ERROR_MSG 59
#define ODCI_EVICT 60
#define ODCI_EXCLUDE 61
#define ODCI_FAILURE_COUNT 62
#define ODCI_FIRST 63
#define ODCI_FPC_SLOT_NUMBER 64
#define ODCI_FQDN 65
#define ODCI_FQDN_LIST 66
#define ODCI_GCON_DISPLAY_NUM 67
#define ODCI_GCON_PORT 68
#define ODCI_GCON_STATE 69
#define ODCI_GET_CMD_RESPONSE 70
#define ODCI_GLOBAL 71
#define ODCI_GRAPHICS 72
#define ODCI_HOSTNAME 73
#define ODCI_HTTP 74
#define ODCI_HTTP_SERVICE 75
#define ODCI_HTTP_SERVICES 76
#define ODCI_HW_ADDRESS 77
#define ODCI_ID 78
#define ODCI_IMAGE 79
#define ODCI_IMAGE_NAME 80
#define ODCI_IMAGES_LIST 81
#define ODCI_INSTALLED_IMAGE 82
#define ODCI_INSTANCE 83
#define ODCI_INTERFACE 84
#define ODCI_INTERFACE_BRIEF 85
#define ODCI_INTERFACE_CONFIGURED 86
#define ODCI_INTERFACE_LIST 87
#define ODCI_INTERFACE_NAME 88
#define ODCI_INTERFACE_SOURCE 89
#define ODCI_INTERFACE_STATS 90
#define ODCI_INTERFACE_TYPE 91
#define ODCI_INTERFACES_DETAIL 92
#define ODCI_IP 93
#define ODCI_IP_ADDRESS 94
#define ODCI_IPV6 95
#define ODCI_ISO_IMAGES 96
#define ODCI_JUNIPER_ROOT_TAG 97
#define ODCI_LAST_BOOT_PARTITION 98
#define ODCI_LAST_END 99
#define ODCI_LAST_START 100
#define ODCI_LAST_TERMINATED 101
#define ODCI_LINK_MON_TIME 102
#define ODCI_LINK_STATE 103
#define ODCI_LOAD1 104
#define ODCI_LOAD15 105
#define ODCI_LOAD5 106
#define ODCI_MATCH 107
#define ODCI_MATCH_TYPE 108
#define ODCI_MEMORY 109
#define ODCI_MFC_ACCESSLOG_LIST 110
#define ODCI_MFC_ALL_STATS 111
#define ODCI_MFC_BACKUP_FILE 112
#define ODCI_MFC_CLUSTER_NAME 113
#define ODCI_MFC_CONFIG 114
#define ODCI_MFC_CONFIG_VERSION 115
#define ODCI_MFC_CRAWLER_DETAIL 116
#define ODCI_MFC_CRAWLER_LIST 117
#define ODCI_MFC_IMG_ASSOCIATION 118
#define ODCI_MFC_PARTITION_INFO 119
#define ODCI_MFC_RUNNING_CONFIG 120
#define ODCI_MFC_SCHEMA_VERSION 121
#define ODCI_MFC_SERVICE_INFO 122
#define ODCI_MFC_STATS 123
#define ODCI_MFC_SYSTEM_DETAILS 124
#define ODCI_MFC_VM_DETAILS 125
#define ODCI_MFC_VM_INTERFACES 126
#define ODCI_MFC_VM_STORAGE 127
#define ODCI_MFC_VM_VOLUME_DETAILS 128
#define ODCI_MGMT_IP_ADDRESS 129
#define ODCI_MGMT_STATUS 130
#define ODCI_MIN_SCHEMA_VERSION 131
#define ODCI_MODE 132
#define ODCI_MODEL 133
#define ODCI_MTU 134
#define ODCI_NAME 135
#define ODCI_NAMESPACE 136
#define ODCI_NETMASK 137
#define ODCI_NEW_FILE_TYPE 138
#define ODCI_NEXT_BOOT_PARTITION 139
#define ODCI_NFS 140
#define ODCI_NON_PRELOADED_FILES 141
#define ODCI_NVSD_INFO 142
#define ODCI_NW_STATUS 143
#define ODCI_OBJECTS 144
#define ODCI_OP_CMD_RESPONSE 145
#define ODCI_OPCMD_STATUS 146
#define ODCI_ORIGIN 147
#define ODCI_ORIGIN_FETCH 148
#define ODCI_ORIGIN_FQDN 149
#define ODCI_ORIGIN_PORT 150
#define ODCI_PARTITION_DETAIL 151
#define ODCI_PINNED_BYTES 152
#define ODCI_PINNED_OBJECTS 153
#define ODCI_POOL_NAME 154
#define ODCI_PRECEDENCE 155
#define ODCI_PRELOADED_FILES 156
#define ODCI_PROFILER_NAME 157
#define ODCI_PROVIDER 158
#define ODCI_QUERY_STRING 159
#define ODCI_RAM 160
#define ODCI_RAM_CACHE 161
#define ODCI_READ 162
#define ODCI_REGEX 163
#define ODCI_REQUESTS 164
#define ODCI_RESOURCE_POOL 165
#define ODCI_RESOURCE_POOL_DETAIL 166
#define ODCI_RESOURCE_POOL_LIST 167
#define ODCI_RX_BY 168
#define ODCI_RX_DISC 169
#define ODCI_RX_ERR 170
#define ODCI_RX_FRAME 171
#define ODCI_RX_MCAST 172
#define ODCI_RX_ORUNS 173
#define ODCI_RX_PA 174
#define ODCI_SAS_DICT_GEN 175
#define ODCI_SATA_DICT_GEN 176
#define ODCI_SCHEMA_VERSION 177
#define ODCI_SECOND 178
#define ODCI_SERVICE 179
#define ODCI_SERVICE_NAME 180
#define ODCI_SERVICE_STATUS 181
#define ODCI_SERVICES 182
#define ODCI_SIZE 183
#define ODCI_SNAME 184
#define ODCI_SOURCE_FILE 185
#define ODCI_SOURCE_FILE_TYPE 186
#define ODCI_SPEED 187
#define ODCI_SSD_DICT_GEN 188
#define ODCI_STANDBY_IMAGE 189
#define ODCI_START_TIME 190
#define ODCI_SYSTEM 191
#define ODCI_SYSTEM_CPU 192
#define ODCI_SYSTEM_MEMORY 193
#define ODCI_TEXT 194
#define ODCI_TEXT_CONFIG 195
#define ODCI_TIER 196
#define ODCI_TIME_USED 197
#define ODCI_TIMESTAMP 198
#define ODCI_TOTAL 199
#define ODCI_TUNNEL 200
#define ODCI_TX_BY 201
#define ODCI_TX_CARRIER 202
#define ODCI_TX_COLLI 203
#define ODCI_TX_DISC 204
#define ODCI_TX_ERR 205
#define ODCI_TX_ORUNS 206
#define ODCI_TX_PA 207
#define ODCI_TX_QUEUELEN 208
#define ODCI_TXTCON_DEVICE 209
#define ODCI_TXTCON_STATE 210
#define ODCI_UP_DEL_TIME 211
#define ODCI_UPTIME 212
#define ODCI_URI_PREFIX 213
#define ODCI_URL 214
#define ODCI_USED 215
#define ODCI_VDISK 216
#define ODCI_VDISK_FMT 217
#define ODCI_VERSION 218
#define ODCI_VIF_ID 219
#define ODCI_VIF_MAC 220
#define ODCI_VIF_MODEL 221
#define ODCI_VIF_NAME 222
#define ODCI_VIF_TYPE 223
#define ODCI_VIP 224
#define ODCI_VIRTUAL_DISKS 225
#define ODCI_VM 226
#define ODCI_VM_INTERFACE 227
#define ODCI_VMNAME 228
#define ODCI_VMSTATUS 229
#define ODCI_VNET 230
#define ODCI_VNET_IF 231
#define ODCI_WRITE 232
#define ODCI_ZEROCONF 233

#define NUM_ODCI_TAGS 233


#ifdef __cplusplus
    }
}
#endif /* __cplusplus */


#endif /* ODL_JUNIPER_MEDIA_FLOW_CONTROLLER_H */
