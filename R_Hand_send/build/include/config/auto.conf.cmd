deps_config := \
	/home/jhb/esp/esp-idf/components/app_trace/Kconfig \
	/home/jhb/esp/esp-idf/components/aws_iot/Kconfig \
	/home/jhb/esp/esp-idf/components/bt/Kconfig \
	/home/jhb/esp/esp-idf/components/esp32/Kconfig \
	/home/jhb/esp/esp-idf/components/ethernet/Kconfig \
	/home/jhb/esp/esp-idf/components/fatfs/Kconfig \
	/home/jhb/esp/esp-idf/components/freertos/Kconfig \
	/home/jhb/esp/esp-idf/components/heap/Kconfig \
	/home/jhb/esp/esp-idf/components/libsodium/Kconfig \
	/home/jhb/esp/esp-idf/components/log/Kconfig \
	/home/jhb/esp/esp-idf/components/lwip/Kconfig \
	/home/jhb/esp/esp-idf/components/mbedtls/Kconfig \
	/home/jhb/esp/esp-idf/components/openssl/Kconfig \
	/home/jhb/esp/esp-idf/components/pthread/Kconfig \
	/home/jhb/esp/esp-idf/components/spi_flash/Kconfig \
	/home/jhb/esp/esp-idf/components/spiffs/Kconfig \
	/home/jhb/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/jhb/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/jhb/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/jhb/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/jhb/dachuang/esp32_send/esp32_src/R_Hand_send/main/Kconfig.projbuild \
	/home/jhb/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/jhb/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
