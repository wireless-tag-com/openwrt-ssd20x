CONFIG_ESP8089 = m

MODULE_NAME = esp8089

ccflags-y += -DHAS_FW

$(MODULE_NAME)-y := esp_debug.o sdio_sif_esp.o esp_io.o \
    esp_file.o esp_main.o esp_sip.o esp_ext.o esp_ctrl.o \
    esp_mac80211.o esp_debug.o esp_utils.o sdio_stub.o

obj-$(CONFIG_ESP8089) := esp8089.o
