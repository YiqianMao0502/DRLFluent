// -*- Mode: C++; -*-
//                            Package   : omniORB
// transportRule.cc           Created on: 21/08/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2013 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//

#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <orbOptions.h>
#include <transportRules.h>
#include <initialiser.h>
#include <SocketCollection.h>
#include <libcWrapper.h>
#include <omniORB4/giopEndpoint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tcp/tcpConnection.h>

OMNI_NAMESPACE_BEGIN(omni)

static transportRules serverRules_;
static transportRules clientRules_;

static char* dumpRuleString(transportRules::RuleActionPair* ra);

/////////////////////////////////////////////////////////////////////////////
transportRules::transportRules() {
}

/////////////////////////////////////////////////////////////////////////////
transportRules::~transportRules() {
  reset();
}

/////////////////////////////////////////////////////////////////////////////
void
transportRules::reset()
{
  RuleActionPairs::iterator i    = pd_rules.begin();
  RuleActionPairs::iterator last = pd_rules.end();

  for (; i != last; ++i) {
    delete (*i);
  }
  pd_rules.erase(pd_rules.begin(),last);
}

/////////////////////////////////////////////////////////////////////////////
transportRules&
transportRules::serverRules() {
  return serverRules_;
}

/////////////////////////////////////////////////////////////////////////////
transportRules&
transportRules::clientRules() {
  return clientRules_;
}

/////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
transportRules::match(const char*       endpoint,
		      CORBA::StringSeq& actions,
		      CORBA::ULong&     priority)
{
  RuleActionPairs::iterator i    = pd_rules.begin();
  RuleActionPairs::iterator last = pd_rules.end();

  while (i != last) {
    if ((*i)->rule_->match(endpoint)) {
      CORBA::ULong max = (*i)->action_.maximum();
      CORBA::ULong len = (*i)->action_.length();
      actions.replace(max,len,(*i)->action_.get_buffer(),0);
      priority = i - pd_rules.begin();
      return 1;
    }
    i++;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
char*
transportRules::dumpRule(CORBA::ULong index)
{
  RuleActionPairs::iterator i    = pd_rules.begin();
  RuleActionPairs::iterator last = pd_rules.end();

  if ((i+index) >= last) return 0;

  return dumpRuleString((*(i+index)));
}


/////////////////////////////////////////////////////////////////////////////
static 
transportRules::RuleTypes*&
ruleTypes()
{
  static transportRules::RuleTypes* ruletypes_ = 0;
  if (!ruletypes_) {
    ruletypes_ = new transportRules::RuleTypes;
  }
  return ruletypes_;
}

/////////////////////////////////////////////////////////////////////////////
void
transportRules::addRuleType(transportRules::RuleType* rt)
{
  ruleTypes()->push_back(rt);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class builtinMatchAllRule : public transportRules::Rule {
public:
  builtinMatchAllRule(const char* address_mask) : 
    transportRules::Rule(address_mask) {}

  ~builtinMatchAllRule() {}

  CORBA::Boolean match(const char*) { return 1; }
};

/////////////////////////////////////////////////////////////////////////////
static char* extractHost(const char* endpoint)
{
  // Returns the host address if there is one in the endpoint string.

  // Skip giop:tcp: or equivalent.
  const char* p = strchr(endpoint, ':');
  if (p) p = strchr(p+1, ':');
  if (p) {
    ++p;
    CORBA::UShort     port;
    CORBA::String_var host = omniURI::extractHostPort(p, port, 0);

    if (LibcWrapper::isip4addr(host)) {
      return host._retn();
    }
    else if (LibcWrapper::isip6addr(host)) {
      // Check if it's IPv4 encapsulated in IPv6
      if (strncasecmp(host, "::ffff:", 7) == 0 &&
	  LibcWrapper::isip4addr((const char*)host + 7)) {

	return CORBA::string_dup((const char*)host + 7);
      }
      return host._retn();
    }
    else {
      // Try to resolve name
      LibcWrapper::AddrInfo_var ai(LibcWrapper::getAddrInfo(host,port));
      if (ai.in())
        return ai->asString();
    }
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
class builtinLocalHostRule : public transportRules::Rule {
public:
  builtinLocalHostRule(const char* address_mask) : 
    transportRules::Rule(address_mask) {}

  ~builtinLocalHostRule() {}

  CORBA::Boolean match(const char* endpoint)
  {
    if (strncmp(endpoint,"giop:unix:",10) == 0) return 1;

    // Otherwise, we want to check if this endpoint matches one of our
    // addresses.
    CORBA::String_var host = extractHost(endpoint);

    if ((const char*)host)  {
      // Get this host's IP addresses and look for a match
      const omnivector<const char*>* ifaddrs;
      ifaddrs = giopTransportImpl::getInterfaceAddress("giop:tcp");
      if (!ifaddrs) return 0;
      {
	omnivector<const char*>::const_iterator i    = ifaddrs->begin();
	omnivector<const char*>::const_iterator last = ifaddrs->end();
	while (i != last) {
	  if (omni::strMatch((*i),host)) return 1;
	  i++;
	}
      }
    }
    return 0; 
  }
};

/////////////////////////////////////////////////////////////////////////////
class builtinIPv4Rule : public transportRules::Rule {
public:
  builtinIPv4Rule(const char* address_mask,
		  CORBA::ULong n, CORBA::ULong m) : 
    transportRules::Rule(address_mask), network_(n), netmask_(m) {}

  ~builtinIPv4Rule() {}

  CORBA::Boolean matchAddr(const char* ipv4)
  {
    if (ipv4 && LibcWrapper::isip4addr(ipv4)) {
      CORBA::ULong address = inet_addr((char*)ipv4);
      return (network_ == (address & netmask_));
    }
    return 0;
  }

  CORBA::Boolean match(const char* endpoint)
  {
    if (strncmp(endpoint,"giop:unix:",10) == 0) {
      // local transport. Does this rule apply to this host's 
      // IP address(es)? 
      const omnivector<const char*>* ifaddrs;
      ifaddrs = giopTransportImpl::getInterfaceAddress("giop:tcp");
      if (!ifaddrs) return 0;
      {
	omnivector<const char*>::const_iterator i    = ifaddrs->begin();
	omnivector<const char*>::const_iterator last = ifaddrs->end();
	while (i != last) {
          if (matchAddr(*i)) return 1;
	  i++;
	}
      }
      return 0;
    }

    CORBA::String_var ipv4 = extractHost(endpoint);
    return matchAddr(ipv4);
  }

private:
  CORBA::ULong network_;
  CORBA::ULong netmask_;
};


/////////////////////////////////////////////////////////////////////////////
#if defined(OMNI_SUPPORT_IPV6)

class builtinIPv6Rule : public transportRules::Rule {
public:
  typedef CORBA::Octet Addr[16];

  builtinIPv6Rule(const char* address_mask,
		  const Addr& n, CORBA::ULong p) : 
    transportRules::Rule(address_mask), prefix_(p)
  {
    for (int i=0; i < 16; ++i)
      network_[i] = n[i];
  }

  ~builtinIPv6Rule() {}

  CORBA::Boolean matchAddr(const char* ipv6)
  {
    if (ipv6 && LibcWrapper::isip6addr(ipv6)) {

      LibcWrapper::AddrInfo_var ai(LibcWrapper::getAddrInfo(ipv6,0));
      if (!ai.in()) return 0;

      sockaddr_in6* sa        = (sockaddr_in6*)ai->addr();
      CORBA::Octet* ip6_bytes = (CORBA::Octet*)&sa->sin6_addr.s6_addr;
      CORBA::ULong  bits      = prefix_;
      CORBA::ULong  i;

      for (i=0; i < 16 && bits > 7; ++i, bits-=8) {
        if (network_[i] != ip6_bytes[i])
          return 0;
      }
      if (bits) {
        CORBA::Octet mask = (0xff << (8 - bits)) & 0xff;
        if ((network_[i] & mask) != (ip6_bytes[i] & mask))
          return 0;
      }
      return 1;
    }
    return 0;
  }

  CORBA::Boolean match(const char* endpoint)
  {
    if (strncmp(endpoint,"giop:unix:",10) == 0) {
      // local transport. Does this rule apply to this host's 
      // IP address(es)? 
      const omnivector<const char*>* ifaddrs;
      ifaddrs = giopTransportImpl::getInterfaceAddress("giop:tcp");
      if (!ifaddrs) return 0;
      {
	omnivector<const char*>::const_iterator i    = ifaddrs->begin();
	omnivector<const char*>::const_iterator last = ifaddrs->end();
	while (i != last) {
          if (matchAddr(*i)) return 1;
	}
      }
      return 0;
    }

    CORBA::String_var ipv6 = extractHost(endpoint);
    return matchAddr(ipv6);
  }

private:
  Addr         network_;
  CORBA::ULong prefix_;
};

#endif


/////////////////////////////////////////////////////////////////////////////
class builtinRuleType : public transportRules::RuleType {
public:
  builtinRuleType() {
    transportRules::addRuleType((transportRules::RuleType*)this);
  }
  virtual ~builtinRuleType() {}

  CORBA::Boolean createRules(const char*             address_mask,
                             const CORBA::StringSeq& actions,
                             transportRules&         tr)
  {
    CORBA::ULong network = 0, netmask = 0;

#if defined(OMNI_SUPPORT_IPV6)
    builtinIPv6Rule::Addr ip6network;
    CORBA::ULong          prefix;
#endif

    if (omni::strMatch(address_mask,"*")) {
      tr.addRule(new builtinMatchAllRule(address_mask), actions);
      return 1;
    }
    else if (omni::strMatch(address_mask, "localhost")) {
      tr.addRule(new builtinLocalHostRule(address_mask), actions);
      return 1;
    }
    else if (parseIPv4AddressMask(address_mask, network, netmask)) {
      tr.addRule(new builtinIPv4Rule(address_mask, network, netmask), actions);
      return 1;
    }
#if defined(OMNI_SUPPORT_IPV6)
    else if (parseIPv6AddressMask(address_mask, ip6network, prefix)) {
      tr.addRule(new builtinIPv6Rule(address_mask, ip6network, prefix),
                 actions);
      return 1;
    }
#endif
    // Try to resolve as a hostname
    CORBA::Boolean added = 0;

    LibcWrapper::AddrInfo_var aiv(LibcWrapper::getAddrInfo(address_mask, 0));
    if (aiv.in()) {
      LibcWrapper::AddrInfo* ai = aiv;

      while (ai) {
        CORBA::String_var addr = ai->asString();
        if (omniORB::trace(20)) {
          omniORB::logger log;
          log << "Name '" << address_mask << "' in transport rule resolved to '"
              << addr << "'.\n";
        }
        added = createRules(addr, actions, tr) || added;

        ai = ai->next();
      }
    }
    return added;
  }

  static CORBA::Boolean parseIPv4AddressMask(const char*   address,
					     CORBA::ULong& network,
					     CORBA::ULong& netmask)
  {
    CORBA::String_var cp(address);
    char* mask = strchr((char*)cp,'/');
    if (mask) {
      *mask = '\0';
      mask++;
    }
    else {
      mask = (char*) "255.255.255.255";
    }

    if (!LibcWrapper::isip4addr(cp)) return 0;
    network = inet_addr((char*)cp);

    if (LibcWrapper::isip4addr(mask)) {
      netmask = inet_addr(mask);
    }
    else {
      char* maske;
      CORBA::ULong prefix = strtoul(mask, &maske, 10);
      if (*maske || prefix > 32)
	return 0;
      netmask = 0xffffffffU << (32 - prefix);
      netmask = htonl(netmask);
    }
    return 1;
  }

#if defined(OMNI_SUPPORT_IPV6)
  static CORBA::Boolean parseIPv6AddressMask(const char*            address,
					     builtinIPv6Rule::Addr& network,
					     CORBA::ULong&          prefix)
  {
    CORBA::String_var cp(address);

    char* mask = strchr((char*)cp,'/');
    if (mask) {
      *mask = '\0';
      mask++;
      char* maske;
      prefix = strtoul(mask, &maske, 10);

      if (!*mask || *maske || prefix > 128)
	return 0;
    }
    else {
      prefix = 128;
    }
    if (!LibcWrapper::isip6addr(cp)) return 0;

    LibcWrapper::AddrInfo_var ai(LibcWrapper::getAddrInfo(cp, 0));
    if (!ai.in())
      return 0;

    sockaddr_in6* sa = (sockaddr_in6*)ai->addr();
    memcpy((void*)&network, (const void*)&(sa->sin6_addr.s6_addr), 16);
    return 1;
  }
#endif

};

static builtinRuleType builtinRuleType_;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
parseAndAddRuleString(transportRules& tr,
		      const char*     rule_string)
{
  CORBA::StringSeq  actions(4);
  CORBA::String_var address_mask;
  CORBA::Boolean    reset_list = 0;
  CORBA::String_var rs(rule_string); // make a copy

  // Extract address mask
  char* p = rs;
  while (isspace(*p))
    p++;

  char* q = p;
  while (!isspace(*p) && *p != '\0')
    p++;

  if (*p == '\0')
    return 0;

  *p = '\0';

  if (*q == '^') {
    reset_list = 1;
    q++;
    if (*q == '\0') return 0;
  }
  address_mask = CORBA::string_dup(q);
  p++;

  // Extract action list, one or more comma separated action.
  // There may also be white spaces between the actions and comma separators.
  while (isspace(*p))
    p++;
  
  q = p;
  
  p = strchr(q,',');
  while (p && p != q) {
    *p = '\0';
    char* t = q;
    while (!isspace(*t) && *t != '\0')
      t++;
    *t = '\0';
    actions.length(actions.length()+1);
    actions[actions.length()-1] = (const char*) q;

    p++;
    while (isspace(*p))
      p++;
    q = p;
    p = strchr(q,',');
  }
  if (*q == ',')
    return 0;
  if (*q != '\0') {
    p = q;
    while (!isspace(*p) && *p != '\0')
      p++;
    if (*p != '\0')
      *p = '\0';
    actions.length(actions.length()+1);
    actions[actions.length()-1] = (const char*) q;
  }

  if (reset_list)
    tr.reset();

  transportRules::RuleTypes*          ruletypes = ruleTypes();
  transportRules::RuleTypes::iterator i         = ruletypes->begin();
  transportRules::RuleTypes::iterator last      = ruletypes->end();

  for (; i != last; ++i) {
    if ((*i)->createRules(address_mask, actions, tr))
      return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
static
char*
dumpRuleString(transportRules::RuleActionPair* ra)
{
  CORBA::StringSeq& ss  = ra->action_;
  CORBA::ULong      len = strlen(ra->rule_->addressMask()) + 1;

  CORBA::ULong i = 0;
  for (; i < ss.length(); i++) {
    len += strlen(ss[i]) + 1;
  }
  
  CORBA::String_var v(CORBA::string_alloc(len));
  sprintf(v,"%s ",ra->rule_->addressMask());

  i = 0;
  while (i < ss.length()) {
    strcat(v,ss[i]);
    i++;
    if (i != ss.length()) strcat(v,",");
  }
  return v._retn();
}


/////////////////////////////////////////////////////////////////////////////
class clientTransportRuleHandler : public orbOptions::Handler {
public:

  clientTransportRuleHandler() : 
    orbOptions::Handler("clientTransportRule",
			"clientTransportRule = <address mask>  [action]+",
			1,
			"-ORBclientTransportRule \"<address mask>  [action]+\"") {}

  void visit(const char* value,
	     orbOptions::Source)  OMNI_THROW_SPEC (orbOptions::BadParam) {

    if (!parseAndAddRuleString(clientRules_, value)) {
      throw orbOptions::BadParam(key(),value,"Unrecognised address mask");
    }
  }

  void dump(orbOptions::sequenceString& result) {
    omnivector<transportRules::RuleActionPair*>
      ::iterator i = clientRules_.pd_rules.begin();
    omnivector<transportRules::RuleActionPair*>
      ::iterator last = clientRules_.pd_rules.end();

    while (i != last) {
      CORBA::String_var v;
      v = dumpRuleString(*i);
      orbOptions::addKVString(key(),v,result);
      i++;
    }
  }
};

static clientTransportRuleHandler clientTransportRuleHandler_;

/////////////////////////////////////////////////////////////////////////////
class serverTransportRuleHandler : public orbOptions::Handler {
public:

  serverTransportRuleHandler() : 
    orbOptions::Handler("serverTransportRule",
			"serverTransportRule = <address mask>  [action]+",
			1,
			"-ORBserverTransportRule \"<address mask>  [action]+\"") {}

  void visit(const char* value,
	     orbOptions::Source) OMNI_THROW_SPEC (orbOptions::BadParam) {

    if (!parseAndAddRuleString(serverRules_, value)) {
      throw orbOptions::BadParam(key(),value,"Unrecognised address mask");
    }
  }

  void dump(orbOptions::sequenceString& result) {
    omnivector<transportRules::RuleActionPair*>
      ::iterator i = serverRules_.pd_rules.begin();
    omnivector<transportRules::RuleActionPair*>
      ::iterator last = serverRules_.pd_rules.end();

    while (i != last) {
      CORBA::String_var v;
      v = dumpRuleString(*i);
      orbOptions::addKVString(key(),v,result);
      i++;
    }
  }
};

static serverTransportRuleHandler serverTransportRuleHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
class omni_transportRules_initialiser : public omniInitialiser {
public:

  omni_transportRules_initialiser() {
    orbOptions::singleton().registerHandler(clientTransportRuleHandler_);
    orbOptions::singleton().registerHandler(serverTransportRuleHandler_);
  }
  virtual ~omni_transportRules_initialiser() {
    transportRules::RuleTypes*& ruletypes = ruleTypes();
    if (ruletypes) {
      delete ruletypes;
      ruletypes = 0;
    }
  }
  void attach() { 
    if (clientRules_.pd_rules.size() == 0) {
      // Add a default rule
      parseAndAddRuleString(clientRules_, "* unix,ssl,tcp");
    }
    if (serverRules_.pd_rules.size() == 0) {
      // Add a default rule
      parseAndAddRuleString(serverRules_, "* unix,ssl,tcp");
    }
  }
  void detach() { 
    serverRules_.reset();
    clientRules_.reset();
  }
};


static omni_transportRules_initialiser initialiser;

omniInitialiser& omni_transportRules_initialiser_ = initialiser;



OMNI_NAMESPACE_END(omni)
