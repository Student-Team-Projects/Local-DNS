CONFIG=./config
CACHE_LOC=/var/cache/local_dns
CONFIG_LOC=/etc/local_dns
PROGRAM_LOC=/usr/local/local_dns
UNIT_FILE_LOC=/etc/systemd/system
LOGS=/var/log/local_dns

PROJECT_DIR = $(shell pwd)
CONFIG = $(PROJECT_DIR)/config
LIB = $(PROJECT_DIR)/lib
LIB_CRAFTER_SRC = $(PROJECT_DIR)/libcrafter/libcrafter
DNS_SERVER=$(PROJECT_DIR)/dns_server
NETWORKING=$(PROJECT_DIR)/networking
HOSTS=$(PROJECT_DIR)/hosts

.PHONY: libcrafter
.PHONY: clean
.PHONY: local_dns
.PHONY: install

udp: $(NETWORKING)/ip_getter.global.o $(NETWORKING)/crafter_requester.global.o $(CONFIG)/DnsMap.global.o $(CONFIG)/DnsMapCache.global.o $(CONFIG)/DnsMapUser.global.o $(CONFIG)/DnsMapUserSettings.global.o $(LIB)/time_utils.o
	g++ -std=c++20 -L/usr/local/lib -Wl,-rpath=/usr/local/lib -DGLOBAL=1 $(DNS_SERVER)/udp.cpp -o $(DNS_SERVER)/udp.x $(NETWORKING)/ip_getter.global.o $(NETWORKING)/crafter_requester.global.o $(CONFIG)/DnsMap.global.o $(CONFIG)/DnsMapCache.global.o $(CONFIG)/DnsMapUser.global.o $(CONFIG)/DnsMapUserSettings.global.o $(LIB)/time_utils.o -pthread -lcrafter -lnsl -lrt -lpcap -lm -lresolv
tcp: $(NETWORKING)/ip_getter.global.o $(NETWORKING)/crafter_requester.global.o $(CONFIG)/DnsMap.global.o $(CONFIG)/DnsMapCache.global.o $(CONFIG)/DnsMapUser.global.o $(CONFIG)/DnsMapUserSettings.global.o $(LIB)/time_utils.o
	g++ -std=c++20 -L/usr/local/lib -Wl,-rpath=/usr/local/lib -DGLOBAL=1 $(DNS_SERVER)/tcp.cpp -o $(DNS_SERVER)/tcp.x $(NETWORKING)/ip_getter.global.o $(NETWORKING)/crafter_requester.global.o $(CONFIG)/DnsMap.global.o $(CONFIG)/DnsMapCache.global.o $(CONFIG)/DnsMapUser.global.o $(CONFIG)/DnsMapUserSettings.global.o $(LIB)/time_utils.o -pthread -lcrafter -lnsl -lrt -lpcap -lm -lresolv

%.global.o : %.cpp
	g++ -DGLOBAL=1 -std=c++20 -I$(LIB_CRAFTER_SRC) -c $^ -o $@

%.o : %.cpp
	g++ -DGLOBAL=0 -std=c++20 -I$(LIB_CRAFTER_SRC) -c $^ -o $@

libcrafter:
	cd $(LIB_CRAFTER_SRC) && ./autogen.sh
	make -C $(LIB_CRAFTER_SRC) all
	make -C $(LIB_CRAFTER_SRC) DESTDIR=$(DESTDIR) install
	ldconfig -n $(DESTDIR)

clean:
	rm -f $(NETWORKING)/*.o
	rm -f $(CONFIG)/*.o
	rm -f $(LIB)/*.o
ifneq ("$(wildcard $(LIB_CRAFTER_SRC))","")
	make -C $(LIB_CRAFTER_SRC) clean
endif
	rm -f $(DNS_SERVER)/*.x


local_dns: libcrafter udp tcp



install:
	mkdir -p $(DESTDIR)$(LOGS)
	mkdir -p $(DESTDIR)$(CACHE_LOC)
	mkdir -p $(DESTDIR)$(CONFIG_LOC)
	cp $(CONFIG)/DnsMapCache.config $(DESTDIR)$(CACHE_LOC)
	cp $(CONFIG)/DnsMapUserSettings.config $(DESTDIR)$(CONFIG_LOC)
	cp $(CONFIG)/DnsMapUser.config $(DESTDIR)$(CONFIG_LOC)
	mkdir -p $(DESTDIR)$(PROGRAM_LOC)
	# cp $(DNS_SERVER)/get_ip.x $(DESTDIR)/$(PROGRAM_LOC)
	cp $(DNS_SERVER)/custom_dns.py $(DESTDIR)/$(PROGRAM_LOC)
	cp $(HOSTS)/hosts_daemon.py $(DESTDIR)/$(PROGRAM_LOC)
	mkdir -p $(DESTDIR)$(UNIT_FILE_LOC)
	cp installation/unit_file $(DESTDIR)$(UNIT_FILE_LOC)/local-dns.service
	cp installation/unit_file_hosts $(DESTDIR)$(UNIT_FILE_LOC)/local-dns-hosts.service

