#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sstar-ffmpeg
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=utils
  DEPENDS:=@TARGET_sstar +sstar-mpp-lib +libpthread +libopenssl +zlib
  TITLE:=SSD201/SD202 ffmpeg
endef

define Package/$(PKG_NAME)/description
 SSD20X ffmpeg
endef

FFMPEG_CONFIGURE := \
	./configure \
	--prefix="/usr" \
	--disable-ffmpeg \
	--disable-ffprobe \
	--disable-ffplay \
	--disable-programs \
	--disable-symver \
	--disable-doc \
	--disable-htmlpages \
	--disable-manpages \
	--disable-podpages \
	--disable-txtpages \
	--disable-avresample \
	--disable-avfilter \
	--disable-avdevice \
	--disable-postproc \
	--enable-swscale \
	--enable-swresample \
	--enable-avcodec \
	--enable-avformat \
	--enable-small \
	--enable-cross-compile \
	--target-os=linux \
	--cross-prefix="$(TARGET_CROSS)" \
	--arch="$(ARCH)" \
	--enable-shared \
	--disable-gpl \
	--enable-nonfree \
	--enable-openssl \
	--enable-pthreads \
	--enable-error-resilience \
	--enable-debug \
	--disable-encoders \
	--disable-hwaccels \
	--disable-muxers \
	--disable-indevs \
	--disable-outdevs \
	--disable-devices \
	--disable-filters \
	--extra-cflags="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -I$(STAGING_DIR)/usr/include/ssd201mpp -rdynamic -funwind-tables -ffunction-sections" \
	--extra-ldflags="$(TARGET_LDFLAGS)"

define Build/Configure
	( cd $(PKG_BUILD_DIR); $(FFMPEG_CONFIGURE) )
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lib{avcodec,avformat,avutil,swscale,swresample} $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avformat,avutil,swscale,swresample}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lib{avcodec,avformat,avutil,swscale,swresample}.pc $(1)/usr/lib/pkgconfig/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{avcodec,avformat,avutil,swscale,swresample}.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
