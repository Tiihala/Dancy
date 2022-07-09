# Dancy Operating System

##############################################################################

ACPICA_OBJECTS_32= \
 ./o32/acpica/dsargs.o \
 ./o32/acpica/dscontrol.o \
 ./o32/acpica/dsdebug.o \
 ./o32/acpica/dsfield.o \
 ./o32/acpica/dsinit.o \
 ./o32/acpica/dsmethod.o \
 ./o32/acpica/dsmthdat.o \
 ./o32/acpica/dsobject.o \
 ./o32/acpica/dsopcode.o \
 ./o32/acpica/dspkginit.o \
 ./o32/acpica/dsutils.o \
 ./o32/acpica/dswexec.o \
 ./o32/acpica/dswload.o \
 ./o32/acpica/dswload2.o \
 ./o32/acpica/dswscope.o \
 ./o32/acpica/dswstate.o \
 ./o32/acpica/evevent.o \
 ./o32/acpica/evglock.o \
 ./o32/acpica/evgpe.o \
 ./o32/acpica/evgpeblk.o \
 ./o32/acpica/evgpeinit.o \
 ./o32/acpica/evgpeutil.o \
 ./o32/acpica/evhandler.o \
 ./o32/acpica/evmisc.o \
 ./o32/acpica/evregion.o \
 ./o32/acpica/evrgnini.o \
 ./o32/acpica/evsci.o \
 ./o32/acpica/evxface.o \
 ./o32/acpica/evxfevnt.o \
 ./o32/acpica/evxfgpe.o \
 ./o32/acpica/evxfregn.o \
 ./o32/acpica/exconcat.o \
 ./o32/acpica/exconfig.o \
 ./o32/acpica/exconvrt.o \
 ./o32/acpica/excreate.o \
 ./o32/acpica/exdebug.o \
 ./o32/acpica/exdump.o \
 ./o32/acpica/exfield.o \
 ./o32/acpica/exfldio.o \
 ./o32/acpica/exmisc.o \
 ./o32/acpica/exmutex.o \
 ./o32/acpica/exnames.o \
 ./o32/acpica/exoparg1.o \
 ./o32/acpica/exoparg2.o \
 ./o32/acpica/exoparg3.o \
 ./o32/acpica/exoparg6.o \
 ./o32/acpica/exprep.o \
 ./o32/acpica/exregion.o \
 ./o32/acpica/exresnte.o \
 ./o32/acpica/exresolv.o \
 ./o32/acpica/exresop.o \
 ./o32/acpica/exserial.o \
 ./o32/acpica/exstore.o \
 ./o32/acpica/exstoren.o \
 ./o32/acpica/exstorob.o \
 ./o32/acpica/exsystem.o \
 ./o32/acpica/extrace.o \
 ./o32/acpica/exutils.o \
 ./o32/acpica/hwacpi.o \
 ./o32/acpica/hwesleep.o \
 ./o32/acpica/hwgpe.o \
 ./o32/acpica/hwpci.o \
 ./o32/acpica/hwregs.o \
 ./o32/acpica/hwsleep.o \
 ./o32/acpica/hwtimer.o \
 ./o32/acpica/hwvalid.o \
 ./o32/acpica/hwxface.o \
 ./o32/acpica/hwxfsleep.o \
 ./o32/acpica/nsaccess.o \
 ./o32/acpica/nsalloc.o \
 ./o32/acpica/nsarguments.o \
 ./o32/acpica/nsconvert.o \
 ./o32/acpica/nsdump.o \
 ./o32/acpica/nsdumpdv.o \
 ./o32/acpica/nseval.o \
 ./o32/acpica/nsinit.o \
 ./o32/acpica/nsload.o \
 ./o32/acpica/nsnames.o \
 ./o32/acpica/nsobject.o \
 ./o32/acpica/nsparse.o \
 ./o32/acpica/nspredef.o \
 ./o32/acpica/nsprepkg.o \
 ./o32/acpica/nsrepair.o \
 ./o32/acpica/nsrepair2.o \
 ./o32/acpica/nssearch.o \
 ./o32/acpica/nsutils.o \
 ./o32/acpica/nswalk.o \
 ./o32/acpica/nsxfeval.o \
 ./o32/acpica/nsxfname.o \
 ./o32/acpica/nsxfobj.o \
 ./o32/acpica/psargs.o \
 ./o32/acpica/psloop.o \
 ./o32/acpica/psobject.o \
 ./o32/acpica/psopcode.o \
 ./o32/acpica/psopinfo.o \
 ./o32/acpica/psparse.o \
 ./o32/acpica/psscope.o \
 ./o32/acpica/pstree.o \
 ./o32/acpica/psutils.o \
 ./o32/acpica/pswalk.o \
 ./o32/acpica/psxface.o \
 ./o32/acpica/rsaddr.o \
 ./o32/acpica/rscalc.o \
 ./o32/acpica/rscreate.o \
 ./o32/acpica/rsdump.o \
 ./o32/acpica/rsdumpinfo.o \
 ./o32/acpica/rsinfo.o \
 ./o32/acpica/rsio.o \
 ./o32/acpica/rsirq.o \
 ./o32/acpica/rslist.o \
 ./o32/acpica/rsmemory.o \
 ./o32/acpica/rsmisc.o \
 ./o32/acpica/rsserial.o \
 ./o32/acpica/rsutils.o \
 ./o32/acpica/rsxface.o \
 ./o32/acpica/tbdata.o \
 ./o32/acpica/tbfadt.o \
 ./o32/acpica/tbfind.o \
 ./o32/acpica/tbinstal.o \
 ./o32/acpica/tbprint.o \
 ./o32/acpica/tbutils.o \
 ./o32/acpica/tbxface.o \
 ./o32/acpica/tbxfload.o \
 ./o32/acpica/tbxfroot.o \
 ./o32/acpica/utaddress.o \
 ./o32/acpica/utalloc.o \
 ./o32/acpica/utascii.o \
 ./o32/acpica/utbuffer.o \
 ./o32/acpica/utcache.o \
 ./o32/acpica/utclib.o \
 ./o32/acpica/utcopy.o \
 ./o32/acpica/utdebug.o \
 ./o32/acpica/utdecode.o \
 ./o32/acpica/utdelete.o \
 ./o32/acpica/uterror.o \
 ./o32/acpica/uteval.o \
 ./o32/acpica/utexcep.o \
 ./o32/acpica/utglobal.o \
 ./o32/acpica/uthex.o \
 ./o32/acpica/utids.o \
 ./o32/acpica/utinit.o \
 ./o32/acpica/utlock.o \
 ./o32/acpica/utmath.o \
 ./o32/acpica/utmisc.o \
 ./o32/acpica/utmutex.o \
 ./o32/acpica/utnonansi.o \
 ./o32/acpica/utobject.o \
 ./o32/acpica/utosi.o \
 ./o32/acpica/utownerid.o \
 ./o32/acpica/utpredef.o \
 ./o32/acpica/utprint.o \
 ./o32/acpica/utresdecode.o \
 ./o32/acpica/utresrc.o \
 ./o32/acpica/utstate.o \
 ./o32/acpica/utstring.o \
 ./o32/acpica/utstrsuppt.o \
 ./o32/acpica/utstrtoul64.o \
 ./o32/acpica/uttrack.o \
 ./o32/acpica/utuuid.o \
 ./o32/acpica/utxface.o \
 ./o32/acpica/utxferror.o \
 ./o32/acpica/utxfinit.o \
 ./o32/acpica/utxfmutex.o \

ACPICA_OBJECTS_64= \
 ./o64/acpica/dsargs.o \
 ./o64/acpica/dscontrol.o \
 ./o64/acpica/dsdebug.o \
 ./o64/acpica/dsfield.o \
 ./o64/acpica/dsinit.o \
 ./o64/acpica/dsmethod.o \
 ./o64/acpica/dsmthdat.o \
 ./o64/acpica/dsobject.o \
 ./o64/acpica/dsopcode.o \
 ./o64/acpica/dspkginit.o \
 ./o64/acpica/dsutils.o \
 ./o64/acpica/dswexec.o \
 ./o64/acpica/dswload.o \
 ./o64/acpica/dswload2.o \
 ./o64/acpica/dswscope.o \
 ./o64/acpica/dswstate.o \
 ./o64/acpica/evevent.o \
 ./o64/acpica/evglock.o \
 ./o64/acpica/evgpe.o \
 ./o64/acpica/evgpeblk.o \
 ./o64/acpica/evgpeinit.o \
 ./o64/acpica/evgpeutil.o \
 ./o64/acpica/evhandler.o \
 ./o64/acpica/evmisc.o \
 ./o64/acpica/evregion.o \
 ./o64/acpica/evrgnini.o \
 ./o64/acpica/evsci.o \
 ./o64/acpica/evxface.o \
 ./o64/acpica/evxfevnt.o \
 ./o64/acpica/evxfgpe.o \
 ./o64/acpica/evxfregn.o \
 ./o64/acpica/exconcat.o \
 ./o64/acpica/exconfig.o \
 ./o64/acpica/exconvrt.o \
 ./o64/acpica/excreate.o \
 ./o64/acpica/exdebug.o \
 ./o64/acpica/exdump.o \
 ./o64/acpica/exfield.o \
 ./o64/acpica/exfldio.o \
 ./o64/acpica/exmisc.o \
 ./o64/acpica/exmutex.o \
 ./o64/acpica/exnames.o \
 ./o64/acpica/exoparg1.o \
 ./o64/acpica/exoparg2.o \
 ./o64/acpica/exoparg3.o \
 ./o64/acpica/exoparg6.o \
 ./o64/acpica/exprep.o \
 ./o64/acpica/exregion.o \
 ./o64/acpica/exresnte.o \
 ./o64/acpica/exresolv.o \
 ./o64/acpica/exresop.o \
 ./o64/acpica/exserial.o \
 ./o64/acpica/exstore.o \
 ./o64/acpica/exstoren.o \
 ./o64/acpica/exstorob.o \
 ./o64/acpica/exsystem.o \
 ./o64/acpica/extrace.o \
 ./o64/acpica/exutils.o \
 ./o64/acpica/hwacpi.o \
 ./o64/acpica/hwesleep.o \
 ./o64/acpica/hwgpe.o \
 ./o64/acpica/hwpci.o \
 ./o64/acpica/hwregs.o \
 ./o64/acpica/hwsleep.o \
 ./o64/acpica/hwtimer.o \
 ./o64/acpica/hwvalid.o \
 ./o64/acpica/hwxface.o \
 ./o64/acpica/hwxfsleep.o \
 ./o64/acpica/nsaccess.o \
 ./o64/acpica/nsalloc.o \
 ./o64/acpica/nsarguments.o \
 ./o64/acpica/nsconvert.o \
 ./o64/acpica/nsdump.o \
 ./o64/acpica/nsdumpdv.o \
 ./o64/acpica/nseval.o \
 ./o64/acpica/nsinit.o \
 ./o64/acpica/nsload.o \
 ./o64/acpica/nsnames.o \
 ./o64/acpica/nsobject.o \
 ./o64/acpica/nsparse.o \
 ./o64/acpica/nspredef.o \
 ./o64/acpica/nsprepkg.o \
 ./o64/acpica/nsrepair.o \
 ./o64/acpica/nsrepair2.o \
 ./o64/acpica/nssearch.o \
 ./o64/acpica/nsutils.o \
 ./o64/acpica/nswalk.o \
 ./o64/acpica/nsxfeval.o \
 ./o64/acpica/nsxfname.o \
 ./o64/acpica/nsxfobj.o \
 ./o64/acpica/psargs.o \
 ./o64/acpica/psloop.o \
 ./o64/acpica/psobject.o \
 ./o64/acpica/psopcode.o \
 ./o64/acpica/psopinfo.o \
 ./o64/acpica/psparse.o \
 ./o64/acpica/psscope.o \
 ./o64/acpica/pstree.o \
 ./o64/acpica/psutils.o \
 ./o64/acpica/pswalk.o \
 ./o64/acpica/psxface.o \
 ./o64/acpica/rsaddr.o \
 ./o64/acpica/rscalc.o \
 ./o64/acpica/rscreate.o \
 ./o64/acpica/rsdump.o \
 ./o64/acpica/rsdumpinfo.o \
 ./o64/acpica/rsinfo.o \
 ./o64/acpica/rsio.o \
 ./o64/acpica/rsirq.o \
 ./o64/acpica/rslist.o \
 ./o64/acpica/rsmemory.o \
 ./o64/acpica/rsmisc.o \
 ./o64/acpica/rsserial.o \
 ./o64/acpica/rsutils.o \
 ./o64/acpica/rsxface.o \
 ./o64/acpica/tbdata.o \
 ./o64/acpica/tbfadt.o \
 ./o64/acpica/tbfind.o \
 ./o64/acpica/tbinstal.o \
 ./o64/acpica/tbprint.o \
 ./o64/acpica/tbutils.o \
 ./o64/acpica/tbxface.o \
 ./o64/acpica/tbxfload.o \
 ./o64/acpica/tbxfroot.o \
 ./o64/acpica/utaddress.o \
 ./o64/acpica/utalloc.o \
 ./o64/acpica/utascii.o \
 ./o64/acpica/utbuffer.o \
 ./o64/acpica/utcache.o \
 ./o64/acpica/utclib.o \
 ./o64/acpica/utcopy.o \
 ./o64/acpica/utdebug.o \
 ./o64/acpica/utdecode.o \
 ./o64/acpica/utdelete.o \
 ./o64/acpica/uterror.o \
 ./o64/acpica/uteval.o \
 ./o64/acpica/utexcep.o \
 ./o64/acpica/utglobal.o \
 ./o64/acpica/uthex.o \
 ./o64/acpica/utids.o \
 ./o64/acpica/utinit.o \
 ./o64/acpica/utlock.o \
 ./o64/acpica/utmath.o \
 ./o64/acpica/utmisc.o \
 ./o64/acpica/utmutex.o \
 ./o64/acpica/utnonansi.o \
 ./o64/acpica/utobject.o \
 ./o64/acpica/utosi.o \
 ./o64/acpica/utownerid.o \
 ./o64/acpica/utpredef.o \
 ./o64/acpica/utprint.o \
 ./o64/acpica/utresdecode.o \
 ./o64/acpica/utresrc.o \
 ./o64/acpica/utstate.o \
 ./o64/acpica/utstring.o \
 ./o64/acpica/utstrsuppt.o \
 ./o64/acpica/utstrtoul64.o \
 ./o64/acpica/uttrack.o \
 ./o64/acpica/utuuid.o \
 ./o64/acpica/utxface.o \
 ./o64/acpica/utxferror.o \
 ./o64/acpica/utxfinit.o \
 ./o64/acpica/utxfmutex.o \

ACPICA_HEADERS= \
 ./include/acpica/platform/acenv.h \
 ./include/acpica/platform/acenvex.h \
 ./include/acpica/acapps.h \
 ./include/acpica/acbuffer.h \
 ./include/acpica/acclib.h \
 ./include/acpica/accommon.h \
 ./include/acpica/acconfig.h \
 ./include/acpica/acconvert.h \
 ./include/acpica/acdebug.h \
 ./include/acpica/acdisasm.h \
 ./include/acpica/acdispat.h \
 ./include/acpica/acevents.h \
 ./include/acpica/acexcep.h \
 ./include/acpica/acglobal.h \
 ./include/acpica/achware.h \
 ./include/acpica/acinterp.h \
 ./include/acpica/aclocal.h \
 ./include/acpica/acmacros.h \
 ./include/acpica/acnames.h \
 ./include/acpica/acnamesp.h \
 ./include/acpica/acobject.h \
 ./include/acpica/acopcode.h \
 ./include/acpica/acoutput.h \
 ./include/acpica/acparser.h \
 ./include/acpica/acpiosxf.h \
 ./include/acpica/acpixf.h \
 ./include/acpica/acpredef.h \
 ./include/acpica/acresrc.h \
 ./include/acpica/acrestyp.h \
 ./include/acpica/acstruct.h \
 ./include/acpica/actables.h \
 ./include/acpica/actbinfo.h \
 ./include/acpica/actbl.h \
 ./include/acpica/actbl1.h \
 ./include/acpica/actbl2.h \
 ./include/acpica/actbl3.h \
 ./include/acpica/actypes.h \
 ./include/acpica/acutils.h \
 ./include/acpica/acuuid.h \
 ./include/acpica/amlcode.h \
 ./include/acpica/amlresrc.h \
 ./include/acpios/acdancy.h \

ACPICA_DEPS=$(DANCY_EXT) $(DANCY_HEADERS) $(ACPICA_HEADERS)
ACPICA_TEMP=./include/acpica/acpi.h

./o32/acpica.at: $(ACPICA_OBJECTS_32) ./o32/acpios.at
	$(DY_LINK) -o$@ -fat $(ACPICA_OBJECTS_32) ./o32/acpios.at

./o64/acpica.at: $(ACPICA_OBJECTS_64) ./o64/acpios.at
	$(DY_LINK) -o$@ -fat $(ACPICA_OBJECTS_64) ./o64/acpios.at

##############################################################################

./o32/acpica/dsargs.o: ./kernel/acpica/dsargs.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsargs.c

./o32/acpica/dscontrol.o: ./kernel/acpica/dscontrol.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dscontrol.c

./o32/acpica/dsdebug.o: ./kernel/acpica/dsdebug.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsdebug.c

./o32/acpica/dsfield.o: ./kernel/acpica/dsfield.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsfield.c

./o32/acpica/dsinit.o: ./kernel/acpica/dsinit.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsinit.c

./o32/acpica/dsmethod.o: ./kernel/acpica/dsmethod.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsmethod.c

./o32/acpica/dsmthdat.o: ./kernel/acpica/dsmthdat.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsmthdat.c

./o32/acpica/dsobject.o: ./kernel/acpica/dsobject.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsobject.c

./o32/acpica/dsopcode.o: ./kernel/acpica/dsopcode.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsopcode.c

./o32/acpica/dspkginit.o: ./kernel/acpica/dspkginit.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dspkginit.c

./o32/acpica/dsutils.o: ./kernel/acpica/dsutils.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dsutils.c

./o32/acpica/dswexec.o: ./kernel/acpica/dswexec.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dswexec.c

./o32/acpica/dswload.o: ./kernel/acpica/dswload.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dswload.c

./o32/acpica/dswload2.o: ./kernel/acpica/dswload2.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dswload2.c

./o32/acpica/dswscope.o: ./kernel/acpica/dswscope.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dswscope.c

./o32/acpica/dswstate.o: ./kernel/acpica/dswstate.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/dswstate.c

./o32/acpica/evevent.o: ./kernel/acpica/evevent.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evevent.c

./o32/acpica/evglock.o: ./kernel/acpica/evglock.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evglock.c

./o32/acpica/evgpe.o: ./kernel/acpica/evgpe.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evgpe.c

./o32/acpica/evgpeblk.o: ./kernel/acpica/evgpeblk.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evgpeblk.c

./o32/acpica/evgpeinit.o: ./kernel/acpica/evgpeinit.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evgpeinit.c

./o32/acpica/evgpeutil.o: ./kernel/acpica/evgpeutil.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evgpeutil.c

./o32/acpica/evhandler.o: ./kernel/acpica/evhandler.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evhandler.c

./o32/acpica/evmisc.o: ./kernel/acpica/evmisc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evmisc.c

./o32/acpica/evregion.o: ./kernel/acpica/evregion.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evregion.c

./o32/acpica/evrgnini.o: ./kernel/acpica/evrgnini.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evrgnini.c

./o32/acpica/evsci.o: ./kernel/acpica/evsci.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evsci.c

./o32/acpica/evxface.o: ./kernel/acpica/evxface.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evxface.c

./o32/acpica/evxfevnt.o: ./kernel/acpica/evxfevnt.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evxfevnt.c

./o32/acpica/evxfgpe.o: ./kernel/acpica/evxfgpe.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evxfgpe.c

./o32/acpica/evxfregn.o: ./kernel/acpica/evxfregn.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/evxfregn.c

./o32/acpica/exconcat.o: ./kernel/acpica/exconcat.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exconcat.c

./o32/acpica/exconfig.o: ./kernel/acpica/exconfig.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exconfig.c

./o32/acpica/exconvrt.o: ./kernel/acpica/exconvrt.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exconvrt.c

./o32/acpica/excreate.o: ./kernel/acpica/excreate.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/excreate.c

./o32/acpica/exdebug.o: ./kernel/acpica/exdebug.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exdebug.c

./o32/acpica/exdump.o: ./kernel/acpica/exdump.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exdump.c

./o32/acpica/exfield.o: ./kernel/acpica/exfield.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exfield.c

./o32/acpica/exfldio.o: ./kernel/acpica/exfldio.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exfldio.c

./o32/acpica/exmisc.o: ./kernel/acpica/exmisc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exmisc.c

./o32/acpica/exmutex.o: ./kernel/acpica/exmutex.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exmutex.c

./o32/acpica/exnames.o: ./kernel/acpica/exnames.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exnames.c

./o32/acpica/exoparg1.o: ./kernel/acpica/exoparg1.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exoparg1.c

./o32/acpica/exoparg2.o: ./kernel/acpica/exoparg2.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exoparg2.c

./o32/acpica/exoparg3.o: ./kernel/acpica/exoparg3.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exoparg3.c

./o32/acpica/exoparg6.o: ./kernel/acpica/exoparg6.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exoparg6.c

./o32/acpica/exprep.o: ./kernel/acpica/exprep.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exprep.c

./o32/acpica/exregion.o: ./kernel/acpica/exregion.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exregion.c

./o32/acpica/exresnte.o: ./kernel/acpica/exresnte.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exresnte.c

./o32/acpica/exresolv.o: ./kernel/acpica/exresolv.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exresolv.c

./o32/acpica/exresop.o: ./kernel/acpica/exresop.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exresop.c

./o32/acpica/exserial.o: ./kernel/acpica/exserial.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exserial.c

./o32/acpica/exstore.o: ./kernel/acpica/exstore.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exstore.c

./o32/acpica/exstoren.o: ./kernel/acpica/exstoren.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exstoren.c

./o32/acpica/exstorob.o: ./kernel/acpica/exstorob.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exstorob.c

./o32/acpica/exsystem.o: ./kernel/acpica/exsystem.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exsystem.c

./o32/acpica/extrace.o: ./kernel/acpica/extrace.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/extrace.c

./o32/acpica/exutils.o: ./kernel/acpica/exutils.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/exutils.c

./o32/acpica/hwacpi.o: ./kernel/acpica/hwacpi.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwacpi.c

./o32/acpica/hwesleep.o: ./kernel/acpica/hwesleep.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwesleep.c

./o32/acpica/hwgpe.o: ./kernel/acpica/hwgpe.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwgpe.c

./o32/acpica/hwpci.o: ./kernel/acpica/hwpci.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwpci.c

./o32/acpica/hwregs.o: ./kernel/acpica/hwregs.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwregs.c

./o32/acpica/hwsleep.o: ./kernel/acpica/hwsleep.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwsleep.c

./o32/acpica/hwtimer.o: ./kernel/acpica/hwtimer.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwtimer.c

./o32/acpica/hwvalid.o: ./kernel/acpica/hwvalid.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwvalid.c

./o32/acpica/hwxface.o: ./kernel/acpica/hwxface.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwxface.c

./o32/acpica/hwxfsleep.o: ./kernel/acpica/hwxfsleep.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/hwxfsleep.c

./o32/acpica/nsaccess.o: ./kernel/acpica/nsaccess.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsaccess.c

./o32/acpica/nsalloc.o: ./kernel/acpica/nsalloc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsalloc.c

./o32/acpica/nsarguments.o: ./kernel/acpica/nsarguments.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsarguments.c

./o32/acpica/nsconvert.o: ./kernel/acpica/nsconvert.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsconvert.c

./o32/acpica/nsdump.o: ./kernel/acpica/nsdump.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsdump.c

./o32/acpica/nsdumpdv.o: ./kernel/acpica/nsdumpdv.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsdumpdv.c

./o32/acpica/nseval.o: ./kernel/acpica/nseval.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nseval.c

./o32/acpica/nsinit.o: ./kernel/acpica/nsinit.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsinit.c

./o32/acpica/nsload.o: ./kernel/acpica/nsload.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsload.c

./o32/acpica/nsnames.o: ./kernel/acpica/nsnames.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsnames.c

./o32/acpica/nsobject.o: ./kernel/acpica/nsobject.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsobject.c

./o32/acpica/nsparse.o: ./kernel/acpica/nsparse.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsparse.c

./o32/acpica/nspredef.o: ./kernel/acpica/nspredef.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nspredef.c

./o32/acpica/nsprepkg.o: ./kernel/acpica/nsprepkg.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsprepkg.c

./o32/acpica/nsrepair.o: ./kernel/acpica/nsrepair.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsrepair.c

./o32/acpica/nsrepair2.o: ./kernel/acpica/nsrepair2.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsrepair2.c

./o32/acpica/nssearch.o: ./kernel/acpica/nssearch.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nssearch.c

./o32/acpica/nsutils.o: ./kernel/acpica/nsutils.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsutils.c

./o32/acpica/nswalk.o: ./kernel/acpica/nswalk.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nswalk.c

./o32/acpica/nsxfeval.o: ./kernel/acpica/nsxfeval.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsxfeval.c

./o32/acpica/nsxfname.o: ./kernel/acpica/nsxfname.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsxfname.c

./o32/acpica/nsxfobj.o: ./kernel/acpica/nsxfobj.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/nsxfobj.c

./o32/acpica/psargs.o: ./kernel/acpica/psargs.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psargs.c

./o32/acpica/psloop.o: ./kernel/acpica/psloop.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psloop.c

./o32/acpica/psobject.o: ./kernel/acpica/psobject.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psobject.c

./o32/acpica/psopcode.o: ./kernel/acpica/psopcode.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psopcode.c

./o32/acpica/psopinfo.o: ./kernel/acpica/psopinfo.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psopinfo.c

./o32/acpica/psparse.o: ./kernel/acpica/psparse.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psparse.c

./o32/acpica/psscope.o: ./kernel/acpica/psscope.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psscope.c

./o32/acpica/pstree.o: ./kernel/acpica/pstree.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/pstree.c

./o32/acpica/psutils.o: ./kernel/acpica/psutils.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psutils.c

./o32/acpica/pswalk.o: ./kernel/acpica/pswalk.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/pswalk.c

./o32/acpica/psxface.o: ./kernel/acpica/psxface.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/psxface.c

./o32/acpica/rsaddr.o: ./kernel/acpica/rsaddr.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsaddr.c

./o32/acpica/rscalc.o: ./kernel/acpica/rscalc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rscalc.c

./o32/acpica/rscreate.o: ./kernel/acpica/rscreate.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rscreate.c

./o32/acpica/rsdump.o: ./kernel/acpica/rsdump.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsdump.c

./o32/acpica/rsdumpinfo.o: ./kernel/acpica/rsdumpinfo.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsdumpinfo.c

./o32/acpica/rsinfo.o: ./kernel/acpica/rsinfo.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsinfo.c

./o32/acpica/rsio.o: ./kernel/acpica/rsio.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsio.c

./o32/acpica/rsirq.o: ./kernel/acpica/rsirq.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsirq.c

./o32/acpica/rslist.o: ./kernel/acpica/rslist.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rslist.c

./o32/acpica/rsmemory.o: ./kernel/acpica/rsmemory.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsmemory.c

./o32/acpica/rsmisc.o: ./kernel/acpica/rsmisc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsmisc.c

./o32/acpica/rsserial.o: ./kernel/acpica/rsserial.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsserial.c

./o32/acpica/rsutils.o: ./kernel/acpica/rsutils.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsutils.c

./o32/acpica/rsxface.o: ./kernel/acpica/rsxface.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/rsxface.c

./o32/acpica/tbdata.o: ./kernel/acpica/tbdata.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbdata.c

./o32/acpica/tbfadt.o: ./kernel/acpica/tbfadt.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbfadt.c

./o32/acpica/tbfind.o: ./kernel/acpica/tbfind.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbfind.c

./o32/acpica/tbinstal.o: ./kernel/acpica/tbinstal.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbinstal.c

./o32/acpica/tbprint.o: ./kernel/acpica/tbprint.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbprint.c

./o32/acpica/tbutils.o: ./kernel/acpica/tbutils.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbutils.c

./o32/acpica/tbxface.o: ./kernel/acpica/tbxface.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbxface.c

./o32/acpica/tbxfload.o: ./kernel/acpica/tbxfload.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbxfload.c

./o32/acpica/tbxfroot.o: ./kernel/acpica/tbxfroot.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/tbxfroot.c

./o32/acpica/utaddress.o: ./kernel/acpica/utaddress.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utaddress.c

./o32/acpica/utalloc.o: ./kernel/acpica/utalloc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utalloc.c

./o32/acpica/utascii.o: ./kernel/acpica/utascii.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utascii.c

./o32/acpica/utbuffer.o: ./kernel/acpica/utbuffer.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utbuffer.c

./o32/acpica/utcache.o: ./kernel/acpica/utcache.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utcache.c

./o32/acpica/utclib.o: ./kernel/acpica/utclib.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utclib.c

./o32/acpica/utcopy.o: ./kernel/acpica/utcopy.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utcopy.c

./o32/acpica/utdebug.o: ./kernel/acpica/utdebug.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utdebug.c

./o32/acpica/utdecode.o: ./kernel/acpica/utdecode.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utdecode.c

./o32/acpica/utdelete.o: ./kernel/acpica/utdelete.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utdelete.c

./o32/acpica/uterror.o: ./kernel/acpica/uterror.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/uterror.c

./o32/acpica/uteval.o: ./kernel/acpica/uteval.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/uteval.c

./o32/acpica/utexcep.o: ./kernel/acpica/utexcep.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utexcep.c

./o32/acpica/utglobal.o: ./kernel/acpica/utglobal.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utglobal.c

./o32/acpica/uthex.o: ./kernel/acpica/uthex.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/uthex.c

./o32/acpica/utids.o: ./kernel/acpica/utids.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utids.c

./o32/acpica/utinit.o: ./kernel/acpica/utinit.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utinit.c

./o32/acpica/utlock.o: ./kernel/acpica/utlock.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utlock.c

./o32/acpica/utmath.o: ./kernel/acpica/utmath.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utmath.c

./o32/acpica/utmisc.o: ./kernel/acpica/utmisc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utmisc.c

./o32/acpica/utmutex.o: ./kernel/acpica/utmutex.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utmutex.c

./o32/acpica/utnonansi.o: ./kernel/acpica/utnonansi.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utnonansi.c

./o32/acpica/utobject.o: ./kernel/acpica/utobject.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utobject.c

./o32/acpica/utosi.o: ./kernel/acpica/utosi.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utosi.c

./o32/acpica/utownerid.o: ./kernel/acpica/utownerid.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utownerid.c

./o32/acpica/utpredef.o: ./kernel/acpica/utpredef.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utpredef.c

./o32/acpica/utprint.o: ./kernel/acpica/utprint.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utprint.c

./o32/acpica/utresdecode.o: ./kernel/acpica/utresdecode.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utresdecode.c

./o32/acpica/utresrc.o: ./kernel/acpica/utresrc.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utresrc.c

./o32/acpica/utstate.o: ./kernel/acpica/utstate.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utstate.c

./o32/acpica/utstring.o: ./kernel/acpica/utstring.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utstring.c

./o32/acpica/utstrsuppt.o: ./kernel/acpica/utstrsuppt.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utstrsuppt.c

./o32/acpica/utstrtoul64.o: ./kernel/acpica/utstrtoul64.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utstrtoul64.c

./o32/acpica/uttrack.o: ./kernel/acpica/uttrack.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/uttrack.c

./o32/acpica/utuuid.o: ./kernel/acpica/utuuid.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utuuid.c

./o32/acpica/utxface.o: ./kernel/acpica/utxface.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utxface.c

./o32/acpica/utxferror.o: ./kernel/acpica/utxferror.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utxferror.c

./o32/acpica/utxfinit.o: ./kernel/acpica/utxfinit.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utxfinit.c

./o32/acpica/utxfmutex.o: ./kernel/acpica/utxfmutex.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpica/utxfmutex.c

./o64/acpica/dsargs.o: ./kernel/acpica/dsargs.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsargs.c

./o64/acpica/dscontrol.o: ./kernel/acpica/dscontrol.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dscontrol.c

./o64/acpica/dsdebug.o: ./kernel/acpica/dsdebug.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsdebug.c

./o64/acpica/dsfield.o: ./kernel/acpica/dsfield.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsfield.c

./o64/acpica/dsinit.o: ./kernel/acpica/dsinit.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsinit.c

./o64/acpica/dsmethod.o: ./kernel/acpica/dsmethod.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsmethod.c

./o64/acpica/dsmthdat.o: ./kernel/acpica/dsmthdat.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsmthdat.c

./o64/acpica/dsobject.o: ./kernel/acpica/dsobject.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsobject.c

./o64/acpica/dsopcode.o: ./kernel/acpica/dsopcode.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsopcode.c

./o64/acpica/dspkginit.o: ./kernel/acpica/dspkginit.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dspkginit.c

./o64/acpica/dsutils.o: ./kernel/acpica/dsutils.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dsutils.c

./o64/acpica/dswexec.o: ./kernel/acpica/dswexec.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dswexec.c

./o64/acpica/dswload.o: ./kernel/acpica/dswload.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dswload.c

./o64/acpica/dswload2.o: ./kernel/acpica/dswload2.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dswload2.c

./o64/acpica/dswscope.o: ./kernel/acpica/dswscope.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dswscope.c

./o64/acpica/dswstate.o: ./kernel/acpica/dswstate.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/dswstate.c

./o64/acpica/evevent.o: ./kernel/acpica/evevent.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evevent.c

./o64/acpica/evglock.o: ./kernel/acpica/evglock.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evglock.c

./o64/acpica/evgpe.o: ./kernel/acpica/evgpe.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evgpe.c

./o64/acpica/evgpeblk.o: ./kernel/acpica/evgpeblk.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evgpeblk.c

./o64/acpica/evgpeinit.o: ./kernel/acpica/evgpeinit.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evgpeinit.c

./o64/acpica/evgpeutil.o: ./kernel/acpica/evgpeutil.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evgpeutil.c

./o64/acpica/evhandler.o: ./kernel/acpica/evhandler.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evhandler.c

./o64/acpica/evmisc.o: ./kernel/acpica/evmisc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evmisc.c

./o64/acpica/evregion.o: ./kernel/acpica/evregion.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evregion.c

./o64/acpica/evrgnini.o: ./kernel/acpica/evrgnini.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evrgnini.c

./o64/acpica/evsci.o: ./kernel/acpica/evsci.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evsci.c

./o64/acpica/evxface.o: ./kernel/acpica/evxface.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evxface.c

./o64/acpica/evxfevnt.o: ./kernel/acpica/evxfevnt.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evxfevnt.c

./o64/acpica/evxfgpe.o: ./kernel/acpica/evxfgpe.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evxfgpe.c

./o64/acpica/evxfregn.o: ./kernel/acpica/evxfregn.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/evxfregn.c

./o64/acpica/exconcat.o: ./kernel/acpica/exconcat.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exconcat.c

./o64/acpica/exconfig.o: ./kernel/acpica/exconfig.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exconfig.c

./o64/acpica/exconvrt.o: ./kernel/acpica/exconvrt.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exconvrt.c

./o64/acpica/excreate.o: ./kernel/acpica/excreate.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/excreate.c

./o64/acpica/exdebug.o: ./kernel/acpica/exdebug.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exdebug.c

./o64/acpica/exdump.o: ./kernel/acpica/exdump.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exdump.c

./o64/acpica/exfield.o: ./kernel/acpica/exfield.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exfield.c

./o64/acpica/exfldio.o: ./kernel/acpica/exfldio.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exfldio.c

./o64/acpica/exmisc.o: ./kernel/acpica/exmisc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exmisc.c

./o64/acpica/exmutex.o: ./kernel/acpica/exmutex.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exmutex.c

./o64/acpica/exnames.o: ./kernel/acpica/exnames.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exnames.c

./o64/acpica/exoparg1.o: ./kernel/acpica/exoparg1.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exoparg1.c

./o64/acpica/exoparg2.o: ./kernel/acpica/exoparg2.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exoparg2.c

./o64/acpica/exoparg3.o: ./kernel/acpica/exoparg3.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exoparg3.c

./o64/acpica/exoparg6.o: ./kernel/acpica/exoparg6.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exoparg6.c

./o64/acpica/exprep.o: ./kernel/acpica/exprep.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exprep.c

./o64/acpica/exregion.o: ./kernel/acpica/exregion.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exregion.c

./o64/acpica/exresnte.o: ./kernel/acpica/exresnte.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exresnte.c

./o64/acpica/exresolv.o: ./kernel/acpica/exresolv.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exresolv.c

./o64/acpica/exresop.o: ./kernel/acpica/exresop.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exresop.c

./o64/acpica/exserial.o: ./kernel/acpica/exserial.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exserial.c

./o64/acpica/exstore.o: ./kernel/acpica/exstore.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exstore.c

./o64/acpica/exstoren.o: ./kernel/acpica/exstoren.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exstoren.c

./o64/acpica/exstorob.o: ./kernel/acpica/exstorob.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exstorob.c

./o64/acpica/exsystem.o: ./kernel/acpica/exsystem.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exsystem.c

./o64/acpica/extrace.o: ./kernel/acpica/extrace.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/extrace.c

./o64/acpica/exutils.o: ./kernel/acpica/exutils.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/exutils.c

./o64/acpica/hwacpi.o: ./kernel/acpica/hwacpi.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwacpi.c

./o64/acpica/hwesleep.o: ./kernel/acpica/hwesleep.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwesleep.c

./o64/acpica/hwgpe.o: ./kernel/acpica/hwgpe.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwgpe.c

./o64/acpica/hwpci.o: ./kernel/acpica/hwpci.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwpci.c

./o64/acpica/hwregs.o: ./kernel/acpica/hwregs.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwregs.c

./o64/acpica/hwsleep.o: ./kernel/acpica/hwsleep.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwsleep.c

./o64/acpica/hwtimer.o: ./kernel/acpica/hwtimer.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwtimer.c

./o64/acpica/hwvalid.o: ./kernel/acpica/hwvalid.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwvalid.c

./o64/acpica/hwxface.o: ./kernel/acpica/hwxface.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwxface.c

./o64/acpica/hwxfsleep.o: ./kernel/acpica/hwxfsleep.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/hwxfsleep.c

./o64/acpica/nsaccess.o: ./kernel/acpica/nsaccess.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsaccess.c

./o64/acpica/nsalloc.o: ./kernel/acpica/nsalloc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsalloc.c

./o64/acpica/nsarguments.o: ./kernel/acpica/nsarguments.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsarguments.c

./o64/acpica/nsconvert.o: ./kernel/acpica/nsconvert.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsconvert.c

./o64/acpica/nsdump.o: ./kernel/acpica/nsdump.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsdump.c

./o64/acpica/nsdumpdv.o: ./kernel/acpica/nsdumpdv.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsdumpdv.c

./o64/acpica/nseval.o: ./kernel/acpica/nseval.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nseval.c

./o64/acpica/nsinit.o: ./kernel/acpica/nsinit.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsinit.c

./o64/acpica/nsload.o: ./kernel/acpica/nsload.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsload.c

./o64/acpica/nsnames.o: ./kernel/acpica/nsnames.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsnames.c

./o64/acpica/nsobject.o: ./kernel/acpica/nsobject.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsobject.c

./o64/acpica/nsparse.o: ./kernel/acpica/nsparse.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsparse.c

./o64/acpica/nspredef.o: ./kernel/acpica/nspredef.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nspredef.c

./o64/acpica/nsprepkg.o: ./kernel/acpica/nsprepkg.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsprepkg.c

./o64/acpica/nsrepair.o: ./kernel/acpica/nsrepair.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsrepair.c

./o64/acpica/nsrepair2.o: ./kernel/acpica/nsrepair2.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsrepair2.c

./o64/acpica/nssearch.o: ./kernel/acpica/nssearch.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nssearch.c

./o64/acpica/nsutils.o: ./kernel/acpica/nsutils.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsutils.c

./o64/acpica/nswalk.o: ./kernel/acpica/nswalk.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nswalk.c

./o64/acpica/nsxfeval.o: ./kernel/acpica/nsxfeval.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsxfeval.c

./o64/acpica/nsxfname.o: ./kernel/acpica/nsxfname.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsxfname.c

./o64/acpica/nsxfobj.o: ./kernel/acpica/nsxfobj.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/nsxfobj.c

./o64/acpica/psargs.o: ./kernel/acpica/psargs.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psargs.c

./o64/acpica/psloop.o: ./kernel/acpica/psloop.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psloop.c

./o64/acpica/psobject.o: ./kernel/acpica/psobject.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psobject.c

./o64/acpica/psopcode.o: ./kernel/acpica/psopcode.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psopcode.c

./o64/acpica/psopinfo.o: ./kernel/acpica/psopinfo.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psopinfo.c

./o64/acpica/psparse.o: ./kernel/acpica/psparse.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psparse.c

./o64/acpica/psscope.o: ./kernel/acpica/psscope.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psscope.c

./o64/acpica/pstree.o: ./kernel/acpica/pstree.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/pstree.c

./o64/acpica/psutils.o: ./kernel/acpica/psutils.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psutils.c

./o64/acpica/pswalk.o: ./kernel/acpica/pswalk.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/pswalk.c

./o64/acpica/psxface.o: ./kernel/acpica/psxface.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/psxface.c

./o64/acpica/rsaddr.o: ./kernel/acpica/rsaddr.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsaddr.c

./o64/acpica/rscalc.o: ./kernel/acpica/rscalc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rscalc.c

./o64/acpica/rscreate.o: ./kernel/acpica/rscreate.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rscreate.c

./o64/acpica/rsdump.o: ./kernel/acpica/rsdump.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsdump.c

./o64/acpica/rsdumpinfo.o: ./kernel/acpica/rsdumpinfo.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsdumpinfo.c

./o64/acpica/rsinfo.o: ./kernel/acpica/rsinfo.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsinfo.c

./o64/acpica/rsio.o: ./kernel/acpica/rsio.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsio.c

./o64/acpica/rsirq.o: ./kernel/acpica/rsirq.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsirq.c

./o64/acpica/rslist.o: ./kernel/acpica/rslist.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rslist.c

./o64/acpica/rsmemory.o: ./kernel/acpica/rsmemory.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsmemory.c

./o64/acpica/rsmisc.o: ./kernel/acpica/rsmisc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsmisc.c

./o64/acpica/rsserial.o: ./kernel/acpica/rsserial.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsserial.c

./o64/acpica/rsutils.o: ./kernel/acpica/rsutils.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsutils.c

./o64/acpica/rsxface.o: ./kernel/acpica/rsxface.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/rsxface.c

./o64/acpica/tbdata.o: ./kernel/acpica/tbdata.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbdata.c

./o64/acpica/tbfadt.o: ./kernel/acpica/tbfadt.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbfadt.c

./o64/acpica/tbfind.o: ./kernel/acpica/tbfind.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbfind.c

./o64/acpica/tbinstal.o: ./kernel/acpica/tbinstal.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbinstal.c

./o64/acpica/tbprint.o: ./kernel/acpica/tbprint.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbprint.c

./o64/acpica/tbutils.o: ./kernel/acpica/tbutils.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbutils.c

./o64/acpica/tbxface.o: ./kernel/acpica/tbxface.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbxface.c

./o64/acpica/tbxfload.o: ./kernel/acpica/tbxfload.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbxfload.c

./o64/acpica/tbxfroot.o: ./kernel/acpica/tbxfroot.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/tbxfroot.c

./o64/acpica/utaddress.o: ./kernel/acpica/utaddress.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utaddress.c

./o64/acpica/utalloc.o: ./kernel/acpica/utalloc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utalloc.c

./o64/acpica/utascii.o: ./kernel/acpica/utascii.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utascii.c

./o64/acpica/utbuffer.o: ./kernel/acpica/utbuffer.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utbuffer.c

./o64/acpica/utcache.o: ./kernel/acpica/utcache.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utcache.c

./o64/acpica/utclib.o: ./kernel/acpica/utclib.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utclib.c

./o64/acpica/utcopy.o: ./kernel/acpica/utcopy.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utcopy.c

./o64/acpica/utdebug.o: ./kernel/acpica/utdebug.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utdebug.c

./o64/acpica/utdecode.o: ./kernel/acpica/utdecode.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utdecode.c

./o64/acpica/utdelete.o: ./kernel/acpica/utdelete.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utdelete.c

./o64/acpica/uterror.o: ./kernel/acpica/uterror.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/uterror.c

./o64/acpica/uteval.o: ./kernel/acpica/uteval.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/uteval.c

./o64/acpica/utexcep.o: ./kernel/acpica/utexcep.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utexcep.c

./o64/acpica/utglobal.o: ./kernel/acpica/utglobal.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utglobal.c

./o64/acpica/uthex.o: ./kernel/acpica/uthex.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/uthex.c

./o64/acpica/utids.o: ./kernel/acpica/utids.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utids.c

./o64/acpica/utinit.o: ./kernel/acpica/utinit.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utinit.c

./o64/acpica/utlock.o: ./kernel/acpica/utlock.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utlock.c

./o64/acpica/utmath.o: ./kernel/acpica/utmath.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utmath.c

./o64/acpica/utmisc.o: ./kernel/acpica/utmisc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utmisc.c

./o64/acpica/utmutex.o: ./kernel/acpica/utmutex.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utmutex.c

./o64/acpica/utnonansi.o: ./kernel/acpica/utnonansi.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utnonansi.c

./o64/acpica/utobject.o: ./kernel/acpica/utobject.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utobject.c

./o64/acpica/utosi.o: ./kernel/acpica/utosi.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utosi.c

./o64/acpica/utownerid.o: ./kernel/acpica/utownerid.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utownerid.c

./o64/acpica/utpredef.o: ./kernel/acpica/utpredef.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utpredef.c

./o64/acpica/utprint.o: ./kernel/acpica/utprint.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utprint.c

./o64/acpica/utresdecode.o: ./kernel/acpica/utresdecode.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utresdecode.c

./o64/acpica/utresrc.o: ./kernel/acpica/utresrc.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utresrc.c

./o64/acpica/utstate.o: ./kernel/acpica/utstate.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utstate.c

./o64/acpica/utstring.o: ./kernel/acpica/utstring.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utstring.c

./o64/acpica/utstrsuppt.o: ./kernel/acpica/utstrsuppt.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utstrsuppt.c

./o64/acpica/utstrtoul64.o: ./kernel/acpica/utstrtoul64.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utstrtoul64.c

./o64/acpica/uttrack.o: ./kernel/acpica/uttrack.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/uttrack.c

./o64/acpica/utuuid.o: ./kernel/acpica/utuuid.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utuuid.c

./o64/acpica/utxface.o: ./kernel/acpica/utxface.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utxface.c

./o64/acpica/utxferror.o: ./kernel/acpica/utxferror.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utxferror.c

./o64/acpica/utxfinit.o: ./kernel/acpica/utxfinit.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utxfinit.c

./o64/acpica/utxfmutex.o: ./kernel/acpica/utxfmutex.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpica/utxfmutex.c

##############################################################################

./include/acpica/platform/acenv.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/platform/acenvex.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acapps.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acbuffer.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acclib.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/accommon.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acconfig.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acconvert.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acdebug.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acdisasm.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acdispat.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acevents.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acexcep.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acglobal.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/achware.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acinterp.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/aclocal.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acmacros.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acnames.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acnamesp.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acobject.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acopcode.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acoutput.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acparser.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acpiosxf.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acpixf.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acpredef.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acresrc.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acrestyp.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acstruct.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actables.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actbinfo.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actbl.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actbl1.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actbl2.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actbl3.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/actypes.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acutils.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/acuuid.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/amlcode.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./include/acpica/amlresrc.h: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

##############################################################################

./kernel/acpica/dsargs.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dscontrol.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsdebug.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsfield.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsinit.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsmethod.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsmthdat.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsobject.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsopcode.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dspkginit.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dsutils.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dswexec.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dswload.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dswload2.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dswscope.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/dswstate.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evevent.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evglock.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evgpe.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evgpeblk.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evgpeinit.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evgpeutil.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evhandler.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evmisc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evregion.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evrgnini.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evsci.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evxface.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evxfevnt.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evxfgpe.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/evxfregn.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exconcat.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exconfig.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exconvrt.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/excreate.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exdebug.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exdump.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exfield.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exfldio.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exmisc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exmutex.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exnames.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exoparg1.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exoparg2.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exoparg3.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exoparg6.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exprep.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exregion.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exresnte.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exresolv.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exresop.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exserial.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exstore.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exstoren.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exstorob.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exsystem.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/extrace.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/exutils.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwacpi.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwesleep.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwgpe.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwpci.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwregs.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwsleep.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwtimer.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwvalid.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwxface.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/hwxfsleep.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsaccess.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsalloc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsarguments.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsconvert.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsdump.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsdumpdv.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nseval.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsinit.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsload.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsnames.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsobject.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsparse.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nspredef.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsprepkg.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsrepair.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsrepair2.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nssearch.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsutils.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nswalk.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsxfeval.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsxfname.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/nsxfobj.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psargs.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psloop.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psobject.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psopcode.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psopinfo.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psparse.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psscope.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/pstree.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psutils.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/pswalk.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/psxface.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsaddr.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rscalc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rscreate.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsdump.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsdumpinfo.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsinfo.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsio.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsirq.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rslist.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsmemory.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsmisc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsserial.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsutils.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/rsxface.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbdata.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbfadt.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbfind.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbinstal.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbprint.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbutils.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbxface.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbxfload.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/tbxfroot.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utaddress.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utalloc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utascii.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utbuffer.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utcache.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utclib.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utcopy.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utdebug.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utdecode.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utdelete.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/uterror.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/uteval.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utexcep.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utglobal.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/uthex.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utids.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utinit.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utlock.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utmath.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utmisc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utmutex.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utnonansi.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utobject.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utosi.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utownerid.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utpredef.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utprint.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utresdecode.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utresrc.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utstate.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utstring.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utstrsuppt.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utstrtoul64.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/uttrack.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utuuid.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utxface.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utxferror.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utxfinit.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

./kernel/acpica/utxfmutex.c: $(ACPICA_TEMP)
	$(ACPICA_SOURCE) "$@"

##############################################################################

$(ACPICA_TEMP): ./bin/dy-patch$(DANCY_EXE)
	$(ACPICA_SOURCE) "$@"
