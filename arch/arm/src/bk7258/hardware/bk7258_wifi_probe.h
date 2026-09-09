/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __ARCH_ARM_SRC_BK7258_WIFI_PROBE_H
#define __ARCH_ARM_SRC_BK7258_WIFI_PROBE_H

#include <stddef.h>
#include <stdint.h>

/* Armino app_ab controller_if/cif_main.h and cif_ipc.h, not the newer
 * controller extension ABI used by the reference STA/SoftAP driver. */

#define WIFI_PROBE_GET_MAC       0x202u
#define WIFI_PROBE_GET_STATUS    0x007u
#define WIFI_PROBE_CONFIRM       0x8000u
#define WIFI_PROBE_FREE          0xf3eef3eeu
#define WIFI_PROBE_BUSY          0xcafebabeu
#define WIFI_PROBE_MAX_LIST      60u

struct wifi_probe_node_s
{
  uint32_t header;
  uint32_t head;
  uint32_t tail;
  uint8_t channel;
  uint8_t count;
  uint16_t reserved;
};

struct wifi_probe_cpdu_s
{
  uint32_t next;
  uint16_t length;
  uint8_t type;
  uint8_t flags;
};

struct wifi_probe_request_s
{
  uint32_t reserved0;
  uint16_t command;
  uint16_t sequence;
  uint16_t reserved1;
  uint16_t length;
};

struct wifi_probe_event_s
{
  uint16_t id;
  uint16_t sequence;
  uint16_t reserved;
  uint16_t length;
  uint32_t pattern;
};

struct wifi_probe_status_s
{
  uint8_t state;
  int8_t rssi;
  char ssid[33];
  char ip[16];
  char mask[16];
  char gateway[16];
  char dns[16];
};

_Static_assert(sizeof(struct wifi_probe_node_s) == 16, "Wi-Fi node ABI");
_Static_assert(sizeof(struct wifi_probe_cpdu_s) == 8, "Wi-Fi CPDU ABI");
_Static_assert(sizeof(struct wifi_probe_request_s) == 12, "Wi-Fi request ABI");
_Static_assert(sizeof(struct wifi_probe_event_s) == 12, "Wi-Fi event ABI");
_Static_assert(sizeof(struct wifi_probe_status_s) == 99, "Legacy status ABI");
_Static_assert(offsetof(struct wifi_probe_status_s, ip) == 35,
               "Legacy status IP offset");

#endif
