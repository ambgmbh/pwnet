DGNLNCE	=	$(EXEODIR)$(DIRSEP)dgnlnce$(EXEFILE)
ifdef STATIC
 ODOORS_LIB =	$(SRC_ROOT)$(DIRSEP)odoors$(DIRSEP)$(LIBPREFIX)ODoors$(LIBFILE)
else
 ODOORS_LIB =	$(SRC_ROOT)$(DIRSEP)odoors$(DIRSEP)$(LIBPREFIX)ODoors$(SOFILE)
endif

all: xpdev $(OBJODIR) $(EXEODIR) $(DGNLNCE)

$(DGNLNCE):	$(XPDEV_LIB) $(ODOORS_LIB)
