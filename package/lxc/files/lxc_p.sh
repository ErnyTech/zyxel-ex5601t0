#!/bin/sh /etc/rc.common

START=58

start()
{
	echo "Starting LXC support ..."
	mount -t tmpfs cgroup /sys/fs/cgroup
	for d in cpuset cpu cpuacct memory devices freezer blkio; do
		mkdir -p /sys/fs/cgroup/$d
		(mount -n -t cgroup -o $d cgroup /sys/fs/cgroup/$d || rm -rf /sys/fs/cgroup/$d || true)
	done
	mkdir -p /var/lib/lxc/
	mkdir -p /lib/lxc/rootfs/
	mount -t tmpfs tmpfs /lib/lxc/rootfs/
	# Add required container directories here
	mkdir -p /lib/lxc/rootfs/dev \
		/lib/lxc/rootfs/proc \
		/lib/lxc/rootfs/sys \
		/lib/lxc/rootfs/var \
		/lib/lxc/rootfs/tmp \
		/lib/lxc/rootfs/bin \
		/lib/lxc/rootfs/etc \
		/lib/lxc/rootfs/lib \
		/lib/lxc/rootfs/libexec \
		/lib/lxc/rootfs/sbin \
		/lib/lxc/rootfs/usr

	return 0;
}


stop()
{
	return 0;
}

