FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://bsp.cfg"
SRC_URI:append = " file://0001-net-phy-dp83867-enable-SGMII-mode-explicitly.patch"
KERNEL_FEATURES:append = " bsp.cfg"
