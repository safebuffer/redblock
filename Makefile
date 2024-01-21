NGX_VERSION := $(shell nginx -v 2>&1 | grep -oP 'nginx/\K[0-9]+\.[0-9]+\.[0-9]+')
OSSL_VERSION=3.0.2
PCRE_VERSION=8.45
BASE_DIR := $(shell pwd)


NGX_SLUG=nginx-$(NGX_VERSION)
OSSL_SLUG=openssl-$(OSSL_VERSION)
PCRE_SLUG=pcre-$(PCRE_VERSION)

VENDOR_FOLDER := $(BASE_DIR)/vendor/
OSSL_FOLDER := $(VENDOR_FOLDER)$(OSSL_SLUG)
NGX_FOLDER := $(VENDOR_FOLDER)$(NGX_SLUG)
PCRE_FOLDER := $(VENDOR_FOLDER)$(PCRE_SLUG)

default: build

clean:
	$(MAKE) -C $(NGX_FOLDER) -f Makefile clean

$(NGX_FOLDER)/Makefile:
	$(MAKE) configure

configure: $(VENDOR_FOLDER)
	cd $(NGX_FOLDER); ./configure --with-compat --with-pcre=$(PCRE_FOLDER) \
			--add-dynamic-module=../..

build: $(NGX_FOLDER) $(OSSL_FOLDER) $(PCRE_FOLDER) $(NGX_FOLDER)/Makefile
	$(MAKE) -C $(NGX_FOLDER) -f Makefile modules
	if [ ! -d objs ]; then mkdir objs; fi
	cp $(NGX_FOLDER)/objs/*.so objs

install: $(VENDOR_FOLDER)
	curl http://nginx.org/download/$(NGX_SLUG).tar.gz -o $(NGX_SLUG).tar.gz
	tar -zxf $(NGX_SLUG).tar.gz -C $(VENDOR_FOLDER)
	rm -rf $(NGX_SLUG).tar.gz
	curl https://www.openssl.org/source/$(OSSL_SLUG).tar.gz -o $(OSSL_SLUG).tar.gz
	tar -zxf $(OSSL_SLUG).tar.gz -C $(VENDOR_FOLDER)
	rm -rf $(OSSL_SLUG).tar.gz
	curl https://ftp.exim.org/pub/pcre/$(PCRE_SLUG).tar.gz -o $(PCRE_SLUG).tar.gz
	tar -zxf $(PCRE_SLUG).tar.gz -C $(VENDOR_FOLDER)
	rm -rf $(PCRE_SLUG).tar.gz

$(VENDOR_FOLDER):
	if [ ! -d vendor ]; then mkdir vendor; fi
