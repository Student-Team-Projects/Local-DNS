=====
Todo:
=====

1. Currently, the service operates on the first active interface from the provided ones (or from the scanned ones, if none provided). Please generalize it to all active interfaces.
2. The file ``tcp.cpp`` has a cryptic ``FIXME`` comment that predates us. However, the service appears to never be used with a configuration in which control flow reaches that file. Please determine whether ``tcp.cpp`` is needed at all (which we highly doubt, since DNS uses UDP for simple queries) and fix it or remove it.
