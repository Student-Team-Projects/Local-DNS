This file contains the record of the investigation into the way linux handles resolving dns queries, and ideas about how to intercept that flow.

## systemd-resolved


https://wiki.archlinux.org/index.php/Systemd-resolved


"systemd-resolved is a systemd service that provides network name resolution to local applications via a D-Bus interface, the resolve NSS service (nss-resolve(8)), and a local DNS stub listener on 127.0.0.53. See systemd-resolved(8) for the usage."

The simplest (and probably the least hacky) way to do it would be to setup a server (that answers dns queries our way) somewhere on the 127.0.0.0/8 range and have it added to systemd-resolved list of DNS-Servers. This might get tricky if systemd-resolved has some sort of logic behind choosing the DNS server it uses.

Another approach would be to attempt to replace systemd-resolved in some way, but this seems more troublesome.

## Adding a local DNS server

Let's suppose we have our local DNS server has IP: A.B.C.D. if we have openresolv (https://wiki.archlinux.org/index.php/Openresolv) installed it suffices to add A.B.C.D to the nameserver list via /etc/resolvconf.conf (resolv_conf_local_only=NO also probably needs to be there).

Our DNS server needs to then answer DNS queries in the following way:
 - if the name should be answered by local-dns, answer
 - if not return an ERROR CODE (https://www.ietf.org/rfc/rfc1035.txt) so that whoever is asking knows to ask a different nameserver, the best code to return seems to be Refused, since it suggests the query was valid but the server chooses not to answer.

 To test things out with this you may use https://github.com/TETRX/dns-local-server