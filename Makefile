DESTDIR_BIN = /usr/local/bin
DESTDIR_DATA = /usr/local/share

make:
	cd src && make
clean:
	rm -f phear editor
install:
	mkdir -p $(DESTDIR_BIN)
	mkdir -p $(DESTDIR_DATA)/phear
	cp phear $(DESTDIR_BIN)
	cp -r data $(DESTDIR_DATA)/phear
	ls -al  $(DESTDIR_BIN)/phear
	ls -ald $(DESTDIR_DATA)/phear
uninstall:
	rm -f $(DESTDIR_BIN)/phear
	rm -rf $(DESTDIR_DATA)/phear
