// -*- Mode: C++; -*-
//                            Package   : omniORB
// transportRule.h            Created on: 21/08/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2013 Apasphere Ltd
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

#ifndef __TRANSPORTRULES_H__
#define __TRANSPORTRULES_H__

#include <omniORB4/CORBA.h>


OMNI_NAMESPACE_BEGIN(omni)

class transportRules {
public:

  ////////////////////////////////////////////////////////////////////////
  static transportRules& serverRules();
  static transportRules& clientRules();

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  class Rule {
  public:
    Rule(const char* address_mask) : addressMask_(address_mask) {}
    virtual ~Rule() {}

    virtual CORBA::Boolean match(const char* endpoint) = 0;

    const char* addressMask() { return addressMask_; }

  private:
    CORBA::String_var addressMask_;
    Rule();
    Rule(const Rule&);
    Rule& operator=(const Rule&);
  };

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  class RuleType {
  public:
    virtual CORBA::Boolean createRules(const char*             address_mask,
                                       const CORBA::StringSeq& actions,
                                       transportRules&         tr) = 0;
    // If <address_mask> is recognised by this RuleType instance, add
    // one or more rules to the transportRules and return true; if not
    // recognised, return false.

    RuleType() {}
    virtual ~RuleType() {}

  private:
    RuleType(const RuleType&);
    RuleType& operator=(const RuleType&);
  };

  typedef omnivector<RuleType*> RuleTypes;

  static void addRuleType(RuleType*);


  ////////////////////////////////////////////////////////////////////////
  CORBA::Boolean match(const char* endpoint,
		       CORBA::StringSeq& actions/* return arg */,
		       CORBA::ULong& priority/* return arg */);
  // Return true if <endpoint> matches one of the transport rules.
  // The action list of the matched rule is returned in <actions>.
  // The index of the matched rule is returned in <priority>.
  // Return false if <endpoint> does not match any rule. In that case
  // <actions and <priority> are not initialised.

  ////////////////////////////////////////////////////////////////////////
  char* dumpRule(CORBA::ULong index);
  // Return the string representation of the rule at <index>. Returns 0
  // if the index is out of range. If the value of <priority> returned
  // by match() is used as <index> in this function, the string representation
  // of the rule that match() matches is returned.


  ////////////////////////////////////////////////////////////////////////
  friend class omni_transportRules_initialiser;
  friend class clientTransportRuleHandler;
  friend class serverTransportRuleHandler;

  struct RuleActionPair {
    RuleActionPair(Rule* r, const CORBA::StringSeq& a)
      : rule_(r), action_(a) {}

    ~RuleActionPair() { 
      if (rule_) delete rule_; 
    }
    Rule*            rule_;
    CORBA::StringSeq action_;
  };

  typedef omnivector<RuleActionPair*> RuleActionPairs;

  inline void addRule(Rule* r, const CORBA::StringSeq& a)
  {
    pd_rules.push_back(new RuleActionPair(r, a));
  }


  transportRules();
  ~transportRules();

  void reset();

private:

  RuleTypes       pd_ruletypes;
  RuleActionPairs pd_rules;

  transportRules(const transportRules&);
  transportRules& operator=(const transportRules&);
};

OMNI_NAMESPACE_END(omni)

#endif // __TRANSPORTRULES_H__
