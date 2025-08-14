include $(TOPDIR)/rules.mk

PKG_NAME:=servo_control
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/servo_control
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Servo Control with Rotary Encoder
  DEPENDS:=+libstdcpp
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/servo_control $(PKG_BUILD_DIR)/servo_control.c
endef

define Package/servo_control/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/servo_control $(1)/usr/bin/
endef

$(eval $(call BuildPackage,servo_control))

