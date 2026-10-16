obj-m += cmb950s.o
KDIR ?= /usr/lib/modules/$(shell uname -r)/build
VID = 0x5131
PID = 0x2007

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

reload:
	sudo rmmod cmb950s && make && sudo insmod cmb950s.ko
qemu:
	qemu-system-x86_64 \
		-enable-kvm -m 2G -cpu host \
		-kernel $(KDIR)/arch/x86/boot/bzImage \
		-append "root=/dev/vda2 rw console=ttyS0 loglevel=8 systemd.zram=0" \
		-drive file=$(PWD)/archvm.qcow2,if=virtio \
		-virtfs local,path=$(PWD),mount_tag=src,security_model=mapped-xattr \
		-device qemu-xhci -device usb-host,vendorid=$(VID),productid=$(PID) \
		-netdev user,id=n0,hostfwd=tcp::2222-:22 -device virtio-net,netdev=n0 \
		-nographic

install-service:
	sudo install -m 755 tools/cmb950sd.py /usr/local/bin/cmb950sd.py
	sudo install -m 644 tools/cmb950s.service /etc/systemd/system/cmb950s.service
	sudo systemctl daemon-reload
	sudo systemctl enable --now cmb950s.service

uninstall-service:
	sudo systemctl disable --now cmb950s.service
	sudo rm -f /etc/systemd/system/cmb950s.service /usr/local/bin/cmb950sd.py
	sudo systemctl daemon-reload
