// EPOS Network Mediator

#ifndef __network_h
#define __network_h

#include <machine/nic.h>
#include <network/initializer.h>

#ifdef __ipv4__
#include <network/ipv4/ip.h>
#include <network/ipv4/tcp.h>
#include <network/ipv4/udp.h>
#include <network/bolinha_protocol.h>
#endif

#ifdef __sp__
#include <network/simple_protocol.h>
#endif

#ifdef __tstp__
#include <network/tstp/tstp.h>
#endif

#endif
