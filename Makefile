MSGFMT = msgfmt

PREFIX=/usr/
#PREFIX=/usr/local/

LOCALES = share/locale
#LIB = lib/python/site-packages
DOC = share/doc/kalternatives
BIN = bin
MAN = share/man/man2


make:
install: 
	
	#install -d $(DESTDIR)$(PREFIX)$(LOCALES)/ca/LC_MESSAGES
	#install -d $(DESTDIR)$(PREFIX)$(LOCALES)/es/LC_MESSAGES	
	install -d $(DESTDIR)$(PREFIX)$(BIN)
	#install -d $(DESTDIR)$(PREFIX)$(LIB)
	install -d $(DESTDIR)$(PREFIX)$(DOC)
	#install -d $(DESTDIR)$(PREFIX)$(MAN)

	#$(MSGFMT) po/es.po -o $(DESTDIR)$(PREFIX)$(LOCALES)/es/LC_MESSAGES/kalternatives.mo
	
	#sh compile.sh
	#cp *.py *.pyo $(DESTDIR)$(PREFIX)$(LIB)
	cp kalternatives $(DESTDIR)$(PREFIX)$(BIN)
	chmod +x $(DESTDIR)$(PREFIX)$(BIN)/kalternatives
	cp  Changelog copyright README TODO 	$(DESTDIR)$(PREFIX)$(DOC)
	#cp docs/kalternatives.2.gz $(DESTDIR)$(PREFIX)$(MAN)
	
uninstall:
	rm -f		$(DESTDIR)$(PREFIX)$(BIN)/kalternatives
	#rm -f		$(DESTDIR)$(PREFIX)$(LIB)/altsmanager.*
	rm -rf 		$(DESTDIR)$(PREFIX)$(DOC)
clean:
	rm *.pyc *.pyo

deb: 
	dpkg-buildpackage -rfakeroot
  
