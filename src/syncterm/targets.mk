SYNCTERM	=	$(EXEODIR)$(DIRSEP)syncterm$(EXEFILE)

all: xpdev-mt ciolib-mt uifc-mt $(MTOBJODIR) $(EXEODIR) $(SYNCTERM)

$(SYNCTERM):	$(XPDEV-MT_LIB) $(CIOLIB-MT) $(UIFCLIB-MT) $(SMBLIB)

