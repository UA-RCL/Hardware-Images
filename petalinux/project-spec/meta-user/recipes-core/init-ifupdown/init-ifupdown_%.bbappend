# The image uses Debian ifupdown, whose dhcp method runs "udhcpc -n" and gives
# up after a few seconds, before the link and the switch port are ready.
# Start udhcpc directly with -b so it keeps retrying in the background.
do_install:append() {
    sed -i -e 's#^iface eth0 inet dhcp$#iface eth0 inet manual\n\tpre-up /sbin/ip link set dev eth0 up\n\tup /sbin/udhcpc -b -p /run/udhcpc.eth0.pid -i eth0\n\tdown [ ! -f /run/udhcpc.eth0.pid ] || kill $(cat /run/udhcpc.eth0.pid)#' ${D}${sysconfdir}/network/interfaces
}
