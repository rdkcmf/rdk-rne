AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = 
AM_CXXFLAGS= $(GST_CFLAGS) $(EGL_CFLAGS) $(GLESV2_CFLAGS)
AM_LDFLAGS=

AM_CFLAGS = $(AM_CXXFLAGS) -Wno-deprecated-declarations

bin_PROGRAMS = nwvidtex

nwvidtex_SOURCES = nwvidtex.cpp

nwvidtex_LDFLAGS= \
   $(AM_LDFLAGS) \
   $(GST_LIBS) \
   $(EGL_LIBS) \
   $(GLESV2_LIBS) \
   -lessos -lgstapp-1.0 \
   -Wl,--allow-shlib-undefined

distcleancheck_listfiles = *-libtool

## IPK Generation Support
IPK_GEN_PATH = $(abs_top_builddir)/ipk
IPK_GEN_STAGING_DIR=$(abs_top_builddir)/staging_dir
IPK_GEN_INPUT  = debian-binary control.tar.gz data.tar.gz
IPK_GEN_OUTPUT = nwvidtex.ipk

package_ipk:
	@echo "Generating IPK Package"
	$(MAKE) prefix=${abs_top_builddir}/staging_dir install
	$(mkdir_p) $(IPK_GEN_STAGING_DIR)/usr/bin/
	cp -af $(IPK_GEN_STAGING_DIR)/bin/* $(IPK_GEN_STAGING_DIR)/usr/bin/
	tar -czvf $(IPK_GEN_PATH)/data.tar.gz -C $(IPK_GEN_STAGING_DIR) usr
	tar -czvf $(IPK_GEN_PATH)/control.tar.gz -C $(IPK_GEN_PATH) control postinst prerm
	cd $(IPK_GEN_PATH) && ar cr $(IPK_GEN_OUTPUT) $(IPK_GEN_INPUT) && cd -
	rm -rf ${abs_top_builddir}/staging_dir
	rm -rf $(abs_top_builddir)/ipk/control.tar.gz
	rm -rf $(abs_top_builddir)/ipk/data.tar.gz
