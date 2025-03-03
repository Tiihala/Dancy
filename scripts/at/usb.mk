# Dancy Operating System

##############################################################################

DANCY_USB_OBJECTS_32= \
 ./o32/kernel/usb/usbfs.o \
 ./o32/kernel/usb/xhci.o \

DANCY_USB_OBJECTS_64= \
 ./o64/kernel/usb/usbfs.o \
 ./o64/kernel/usb/xhci.o \

##############################################################################

./o32/usb.at: $(DANCY_USB_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_USB_OBJECTS_32)

./o64/usb.at: $(DANCY_USB_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_USB_OBJECTS_64)
