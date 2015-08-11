# -*- makefile -*-
# package documentation

XSLBASE  = http://docbook.sourceforge.net/release/xsl/current
MANXSL   = $(XSLBASE)/manpages/docbook.xsl
XHTMLXSL = $(XSLBASE)/xhtml/docbook.xsl

% : %.xml
	xsltproc --xinclude $(XHTMLXSL) $< | lynx -stdin -dump -nolist > $@

%.1 : %.xml ../docbook
	xsltproc --xinclude $(MANXSL) $<

%.5 : %.xml ../docbook
	xsltproc --xinclude $(MANXSL) $<
