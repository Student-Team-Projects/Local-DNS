CONFIG=./config
CACHE_LOC=/var/cache/local-dns
CONFIG_LOC=/etc/local-dns
PROGRAM_LOC=/usr/bin
UNIT_FILE_LOC=/usr/lib/systemd/system
LOGS=/var/log/local-dns

PROJECT_DIR = $(shell pwd)
CONFIG = $(PROJECT_DIR)/config
DNS_SERVER=$(PROJECT_DIR)/dns_server

.PHONY: clean
.PHONY: local-dns
.PHONY: install

# Build the main executable
main: $(DNS_SERVER)/udp.o $(DNS_SERVER)/database.o \
	  $(DNS_SERVER)/networking.o $(DNS_SERVER)/settings.o
	g++ -std=c++20 -L/usr/local/lib -Wl,-rpath=/usr/local/lib \
	$(DNS_SERVER)/main.cpp -o $(DNS_SERVER)/local-dns \
	$(DNS_SERVER)/udp.o $(DNS_SERVER)/database.o \
	$(DNS_SERVER)/networking.o $(DNS_SERVER)/settings.o \
	-pthread -lcrafter 

# Compile regular object files
%.o : %.cpp
	g++ -DGLOBAL=0 -std=c++20 -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(DNS_SERVER)/*.o
	rm -f $(DNS_SERVER)/local-dns

# Build everything
local-dns: main

# Install files to DESTDIR
install:
	mkdir -p $(DESTDIR)$(LOGS)
	mkdir -p $(DESTDIR)$(CACHE_LOC)
	mkdir -p $(DESTDIR)$(CONFIG_LOC)
	mkdir -p $(DESTDIR)$(PROGRAM_LOC)
	cp $(CONFIG)/DnsMapUserSettings.config $(DESTDIR)$(CONFIG_LOC)/DnsMapUserSettings.config
	cp $(CONFIG)/DnsMapUser.config $(DESTDIR)$(CONFIG_LOC)/DnsMapUser.config
	cp $(DNS_SERVER)/local-dns $(DESTDIR)$(PROGRAM_LOC)/local-dns
	mkdir -p $(DESTDIR)$(UNIT_FILE_LOC)
	cp installation/unit_file $(DESTDIR)$(UNIT_FILE_LOC)/local-dns.service
