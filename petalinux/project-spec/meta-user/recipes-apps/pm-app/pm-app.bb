#
# Userspace driver for the PE-array program manager (program_manager_top).
#
SUMMARY = "PE-array program manager userspace driver"
DESCRIPTION = "Interactive shell for program_manager_top: uploads the cluster and \
kernel residency tables from /etc/pm-layout.conf, streams instructions into the \
array through the AXI DMA, issues RESET/RUN commands over a /dev/mem mapping and \
prints the PE occupancy map."
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# readline gives the prompt line editing and history.
DEPENDS = "readline"

SRC_URI = "file://pm-app.c \
           file://layout.c \
           file://layout.h \
           file://dma.c \
           file://dma.h \
           file://pm_layout.h \
           file://pm-layout.conf \
           file://Makefile \
          "

# Scarthgap (Yocto 5.0) unpacks file:// sources into UNPACKDIR
# (${WORKDIR}/sources-unpack); older releases put them straight in WORKDIR.
# The ?= keeps this recipe correct on both.
UNPACKDIR ?= "${WORKDIR}"
S = "${UNPACKDIR}"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/pm-app ${D}${bindir}

    # The layout is read at runtime, so editing it on the target and restarting
    # pm-app is enough to re-cluster the grid and move kernels around it -- no
    # rebuild.
    install -d ${D}${sysconfdir}
    install -m 0644 ${S}/pm-layout.conf ${D}${sysconfdir}/pm-layout.conf
}

CONFFILES:${PN} = "${sysconfdir}/pm-layout.conf"
