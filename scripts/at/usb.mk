# Dancy Operating System

##############################################################################

DANCY_USB_OBJECTS_32= \
 ./o32/kernel/usb/boot_kbd.o \
 ./o32/kernel/usb/boot_mse.o \
 ./o32/kernel/usb/ehci.o \
 ./o32/kernel/usb/ohci.o \
 ./o32/kernel/usb/uhci.o \
 ./o32/kernel/usb/usb_ep.o \
 ./o32/kernel/usb/usb_hid.o \
 ./o32/kernel/usb/usb_mnt.o \
 ./o32/kernel/usb/usb_msc.o \
 ./o32/kernel/usb/usb_node.o \
 ./o32/kernel/usb/usb_task.o \
 ./o32/kernel/usb/xhci.o \

DANCY_USB_OBJECTS_64= \
 ./o64/kernel/usb/boot_kbd.o \
 ./o64/kernel/usb/boot_mse.o \
 ./o64/kernel/usb/ehci.o \
 ./o64/kernel/usb/ohci.o \
 ./o64/kernel/usb/uhci.o \
 ./o64/kernel/usb/usb_ep.o \
 ./o64/kernel/usb/usb_hid.o \
 ./o64/kernel/usb/usb_mnt.o \
 ./o64/kernel/usb/usb_msc.o \
 ./o64/kernel/usb/usb_node.o \
 ./o64/kernel/usb/usb_task.o \
 ./o64/kernel/usb/xhci.o \

##############################################################################

./o32/usb.at: $(DANCY_USB_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_USB_OBJECTS_32)

./o64/usb.at: $(DANCY_USB_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_USB_OBJECTS_64)
