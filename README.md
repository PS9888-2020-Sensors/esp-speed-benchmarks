These tests were conducted in a standard home environment (Wi-Fi interference), with two ESP32-DevkitC approximately 1m apart. 250 bytes of header + random data was transmitted continuously for 3s, with the slave device reporting the amount of bytes received every second.

The project was built with `ESP-IDF v4.2-dev-1856-g00148cd0c-dirty` (see `env.sh` for Docker image used).

| Transmission Type | Wi-Fi Data Rate | Max Throughput Observed (bytes) in 1s |
| ----------------- | ----------------| -----------------------|
| Unicast   | `WIFI_PHY_RATE_1M_L`  | 89250 |
| Broadcast | `WIFI_PHY_RATE_1M_L`  | 101000 |
| Unicast   | `WIFI_PHY_RATE_11M_L` | 349250 |
| Broadcast | `WIFI_PHY_RATE_11M_L` | 497000 |
| Unicast   | `WIFI_PHY_RATE_54M`   | 778500 |
| Broadcast | `WIFI_PHY_RATE_54M`   | 963000 |
| Unicast   | `WIFI_PHY_RATE_MCS0_LGI` | 442000 |
| Broadcast | `WIFI_PHY_RATE_MCS0_LGI` | 502250 |
| Unicast   | `WIFI_PHY_RATE_MCS4_LGI` | 756750 |
| Broadcast | `WIFI_PHY_RATE_MCS4_LGI` | 930750 |
| Unicast   | `WIFI_PHY_RATE_MCS7_LGI` | 224750 |
| Broadcast | `WIFI_PHY_RATE_MCS7_LGI` | 706250 |
| Unicast   | `WIFI_PHY_RATE_MCS0_SGI` | 464750 |
| Broadcast | `WIFI_PHY_RATE_MCS0_SGI` | 535000 |
| Unicast   | `WIFI_PHY_RATE_MCS4_SGI` | 771750 |
| Broadcast | `WIFI_PHY_RATE_MCS4_SGI` | 945250 |
| Unicast   | `WIFI_PHY_RATE_MCS7_SGI` | 734250 |
| Broadcast | `WIFI_PHY_RATE_MCS7_SGI` | 833500 |
