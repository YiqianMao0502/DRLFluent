// -*- Mode: C++; -*-
//                          Package   : catior
// catior.cc                Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 2013 Apasphere Ltd
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of catior.
//
//  Catior is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see http://www.gnu.org/licenses/
//
//
// Lists contents of an IOR.

// For vendor tags, see:
// 
//   http://doc.omg.org/vendor-tags


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <omniORB4/CORBA.h>
#include <omniORB4/IOP.h>
#include <omniORB4/omniURI.h>

#ifdef HAVE_STD
#  include <iostream>
#  include <iomanip>
   using namespace std;
#else
#  include <iostream.h>
#  include <iomanip.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

// unistd.h on some platforms, Mac OS X 10.4 for example, defines a
// macro called minor.
#undef minor

#ifndef Swap16
#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))
#else
#error "Swap16 has already been defined"
#endif

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif


OMNI_USING_NAMESPACE(omni)


static void usage(char* progname)
{
  cerr << "usage: " << progname << " [-x] [-o] <stringified IOR>" << endl;
  cerr << "  flag:" << endl;
  cerr << "     -x print object key as a hexadecimal value." << endl;
  cerr << "     -o print extra info for omniORB IOR." << endl;
}


#ifdef HAVE_GETOPT

extern char* optarg;
extern int optind;

#else

// WIN32 doesn't have an implementation of getopt() -
// supply a getopt() for this program:

char* optarg;
int optind = 1;

int
getopt(int num_args, char* const* args, const char* optstring)
{
  if (optind == num_args) return -1;
  char* buf_left = *(args+optind);

  if ((*buf_left != '-' && *buf_left != '/') || buf_left == NULL)
    return -1;

  else if ((optind < (num_args-1)) && strcmp(buf_left,"-") == 0 &&
	   strcmp(*(args+optind+1),"-") == 0) {
    optind+=2;
    return -1;
  }
  else if (strcmp(buf_left,"-") == 0) {
    optind++;
    return '?';
  }

  for (int count = 0; count < strlen(optstring); count++) {
    if (optstring[count] == ':')
      continue;

    if (buf_left[1] == optstring[count]) {
      if (optstring[count+1] == ':') {
        if (strlen(buf_left) > 2) {
          optarg = (buf_left+2);
          optind++;
        }
        else if (optind < (num_args-1)) {
          optarg = *(args+optind+1);
          optind+=2;
        }
        else {
          optind++;
          return '?';
        }
      } else optind++;

      return buf_left[1];
    }
  }
  optind++;
  return '?';
}

#endif  // !HAVE_GETOPT


#define POA_NAME_SEP            '\xff'
#define TRANSIENT_SUFFIX_SEP    '\xfe'
#define TRANSIENT_SUFFIX_SIZE   8


static inline void
print_char(unsigned char c)
{
  if (c >= ' ' && c <= '~') {
    cout << c;
  }
  else {
    char buf[5];
    sprintf(buf, "\\x%02x", (int)c);
    cout << buf;
  }
}

static void
print_key(const _CORBA_Unbounded_Sequence_Octet& key, int hexflag)
{
  if (hexflag) {
    // Output key in hexadecimal form.

    cout << "0x";

    for (unsigned j = 0; j < key.length(); j++) {
      int v = (key[j] & 0xf0) >> 4;

      if (v < 10)
	cout << (char)('0' + v);
      else
	cout << (char)('a' + (v - 10));

      v = key[j] & 0xf;

      if (v < 10)
	cout << (char)('0' + v);
      else
	cout << (char)('a' + (v - 10));
    }

    cout << "  (" << key.length()
	 << " bytes)" << endl;
  }
  else {
    // Output key as text

    cout << "\"";

    for (unsigned j = 0; j < key.length(); j++)
      print_char(key[j]);

    cout << "\"" << endl;
  }
}


static int
get_poa_info(const _CORBA_Unbounded_Sequence_Octet& key,
	     _CORBA_Unbounded_Sequence_String&      poas_out,
	     int&                                   transient_out,
	     _CORBA_Unbounded_Sequence_Octet&       id_out)
{
  const char* k = (const char*) key.NP_data();
  int len = key.length();
  const char* kend = k + len;

  poas_out.length(1);
  poas_out[0] = CORBA::string_dup("root");

  if (*k != TRANSIENT_SUFFIX_SEP && *k != POA_NAME_SEP)  return 0;

  while (k < kend && *k == POA_NAME_SEP) {

    k++;
    const char* name = k;

    while (k < kend && *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP)
      k++;

    if (k == kend)  return 0;

    char* nm = new char[k - name + 1];
    memcpy(nm, name, k - name);
    nm[k - name] = '\0';
    poas_out.length(poas_out.length() + 1);
    poas_out[poas_out.length() - 1] = nm;
  }

  if (k == kend)  return 0;

  transient_out = 0;
  if (*k == TRANSIENT_SUFFIX_SEP) {
    transient_out = 1;
    k += TRANSIENT_SUFFIX_SIZE + 1;
  }
  if (k >= kend || *k)  return 0;
  k++;

  id_out.length(kend - k);
  memcpy(id_out.NP_data(), k, kend - k);

  return 1;
}


static void
print_omni_key(const _CORBA_Unbounded_Sequence_Octet& key, int hexflag)
{
  _CORBA_Unbounded_Sequence_String poas;
  int is_transient;
  _CORBA_Unbounded_Sequence_Octet id;

  if (get_poa_info(key, poas, is_transient, id)) {
    cout << "POA(" << (char*)poas[0];
    for (unsigned i = 1; i < poas.length(); i++)
      cout << '/' << (char*)poas[i];
    cout << ") ";
  }
  else if (key.length() == sizeof(omniOrbBoaKey)) {
    cout << "BOA ";
  }
  else {
    cout << "Unknown ";
  }
  print_key(key, hexflag);
}


//
// TaggedComponents

static
void
print_tag_orb_type(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::ULong orb_type;
  orb_type <<= e;

  cout << "      TAG_ORB_TYPE ";

  switch (orb_type & 0xffffff00) {

  case 0x41545400:
    cout << "omniORB";
    break;

  case 0x48500000:
  case 0x4e534400:
    cout << "HP";
    break;

  case 0x49424d00:
    cout << "IBM";
    break;

  case 0x53554e00:
    cout << "Sun";
    break;

  case 0x4f424200:
  case 0x42454100:
  case 0x574C5300:
    cout << "BEA";
    break;

  case 0x494c5500:
    cout << "Xerox";
    break;

  case 0x58505300:
  case 0x50544300:
    cout << "PrismTech";
    break;

  case 0x49534900:
    cout << "AdNovum Informatik";
    break;

  case 0x56495300:
    cout << "Borland";
    break;

  case 0x4f495300:
    cout << "Object Interface Systems";
    break;

  case 0x46420000:
    cout << "FloorBoard Software";
    break;

  case 0x4e4e4e00:
    cout << "Rogue Wave";
    break;

  case 0x4e550000:
    cout << "Nihon Unisys";
    break;

  case 0x4a424b00:
    cout << "SilverStream Software";
    break;

  case 0x54414f00:
    cout << "TAO";
    break;

  case 0x4c434200:
    cout << "2AB";
    break;

  case 0x41505800:
    cout << "Univ. of Erlangen-Nuernberg";
    break;

  case 0x4f425400:
    cout << "ORBit";
    break;

  case 0x47534900:
    cout << "GemStone Systems";
    break;

  case 0x464a0000:
    cout << "Fujitsu";
    break;

  case 0x4f425f00:
    cout << "TIBCO";
    break;

  case 0x4f414b00:
    cout << "Camros Corporation";
    break;

  case 0x4f4f4300:
    cout << "IONA (Orbacus)";
    break;

  case 0x49545f00:
    cout << "IONA (Orbix)";
    break;

  case 0x4e454300:
    cout << "NEC";
    break;

  case 0x424c5500:
    cout << "Berry Software";
    break;

  case 0x56495400:
    cout << "Vitria";
    break;

  case 0x444f4700:
    cout << "Exoffice Technologies";
    break;

  case 0xcb0e0000:
    cout << "Chicago Board of Exchange";
    break;

  case 0x4a414300:
    cout << "JacORB";
    break;

  case 0x58545200:
    cout << "Xtradyne Technologies";
    break;

  case 0x54475800:
    cout << "Top Graph'X";
    break;

  case 0x41646100:
    cout << "AdaOS Project";
    break;

  case 0x4e4f4b00:
    cout << "Nokia";
    break;

  case 0x45524900:
    cout << "Ericsson";
    break;

  case 0x52415900:
    cout << "RayORB";
    break;

  case 0x53414e00:
    cout << "Sankhya Technologies";
    break;

  case 0x414e4400:
    cout << "Androsoft";
    break;

  case 0x42424300:
    cout << "Bionic Buffalo";
    break;

  case 0x522e4300:
    cout << "Remoting.Corba";
    break;

  case 0x504f0000:
    cout << "PolyORB";
    break;

  case 0x54494400:
    cout << "Telefonica";
    break;

  default:
    cout << "(unknown)";
  }
  cout << " (";
  print_char((orb_type & 0xff000000) >> 24);
  print_char((orb_type & 0x00ff0000) >> 16);
  print_char((orb_type & 0x0000ff00) >>  8);
  print_char((orb_type & 0x000000ff)      );

  cout << ')' << endl;
}



static
void
print_codeset_name(CORBA::ULong id)
{
  switch (id) {

  case 0x00010001: cout << "ISO-8859-1";    break;
  case 0x00010002: cout << "ISO-8859-2";    break;
  case 0x00010003: cout << "ISO-8859-3";    break;
  case 0x00010004: cout << "ISO-8859-4";    break;
  case 0x00010005: cout << "ISO-8859-5";    break;
  case 0x00010006: cout << "ISO-8859-6";    break;
  case 0x00010007: cout << "ISO-8859-7";    break;
  case 0x00010008: cout << "ISO-8859-8";    break;
  case 0x00010009: cout << "ISO-8859-9";    break;
  case 0x0001000a: cout << "ISO-8859-10";   break;
  case 0x0001000b: cout << "ISO-8859-11";   break;
  case 0x0001000d: cout << "ISO-8859-13";   break;
  case 0x0001000e: cout << "ISO-8859-14";   break;
  case 0x0001000f: cout << "ISO-8859-15";   break;
  case 0x00010010: cout << "ISO-8859-16";   break;
  case 0x00010020: cout << "ISO-646";       break;
  case 0x00010100: cout << "UCS-2-level-1"; break;
  case 0x00010101: cout << "UCS-2-level-2"; break;
  case 0x00010102: cout << "UCS-2-level-3"; break;
  case 0x00010106: cout << "UCS-4";         break;
  case 0x05010001: cout << "UTF-8";         break;
  case 0x00010109: cout << "UTF-16";        break;
  case 0x100204e2: cout << "windows-1250";  break;
  case 0x100204e3: cout << "windows-1251";  break;
  case 0x100204e4: cout << "windows-1252";  break;
  case 0x100204e5: cout << "windows-1253";  break;
  case 0x100204e6: cout << "windows-1254";  break;
  case 0x100204e7: cout << "windows-1255";  break;
  case 0x100204e8: cout << "windows-1256";  break;
  case 0x100204e9: cout << "windows-1257";  break;
  case 0x100204ea: cout << "windows-1258";  break;
  case 0x10020025: cout << "IBM-037";       break;
  case 0x100201f8: cout << "IBM-500";       break;
  case 0x10040366: cout << "SNI-EDF-4";     break;
  case 0x10020567: cout << "GBK";           break;

  default:
    {
      char buf[9];
      sprintf(buf, "%08lx", (unsigned long)id);
      cout << "(0x" << buf << ")";
    }
  }
}


static
void
print_tag_code_sets(const IOP::TaggedComponent& c)
{
  CORBA::ULong idx;

  cdrEncapsulationStream e(c.component_data, 1);

  CONV_FRAME::CodeSetComponentInfo info;
  info <<= e;

  cout << "      TAG_CODE_SETS char native code set:       ";
  print_codeset_name(info.ForCharData.native_code_set);
  cout << endl;

  cout << "                    char conversion code sets:  ";

  for (idx=0; idx != info.ForCharData.conversion_code_sets.length(); ++idx) {
    if (idx != 0)
      cout << ", ";

    print_codeset_name(info.ForCharData.conversion_code_sets[idx]);
  }
  cout << endl;

  cout << "                    wchar native code set:      ";
  print_codeset_name(info.ForWcharData.native_code_set);
  cout << endl;

  cout << "                    wchar conversion code sets: ";

  for (idx=0; idx != info.ForWcharData.conversion_code_sets.length(); ++idx) {
    if (idx != 0)
      cout << ", ";

    print_codeset_name(info.ForWcharData.conversion_code_sets[idx]);
  }
  cout << endl << endl;
}

static
void
print_tag_policies(IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  // Encapsulation contains a Messaging::PolicyValueSeq, but the ORB
  // core does not have the stubs for the Messaging module, so we
  // unpick it by hand.

  CORBA::ULong seq_len;
  seq_len <<= e;

  for (CORBA::ULong idx=0; idx != seq_len; ++idx) {
    if (idx == 0)
      cout << "      TAG_POLICIES ";
    else
      cout << "                   ";

    CORBA::ULong    ptype;
    CORBA::OctetSeq pvalue;

    ptype  <<= e;
    pvalue <<= e;

    switch (ptype) {
    case /*ZIOP::COMPRESSION_ENABLING_POLICY_ID*/ 64:
      {
        cdrEncapsulationStream pe(pvalue, 1);
        CORBA::Boolean enabled = pe.unmarshalBoolean();

        cout << "ZIOP::COMPRESSION_ENABLING_POLICY_ID: "
             << (enabled ? "true" : "false")
             << endl;
      }
      break;

    case /*ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID*/ 65:
      {
        cdrEncapsulationStream pe(pvalue, 1);

        cout << "ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID: "
             << endl;

        // Encapsulation contains a Compression::CompressorIdLevelList

        CORBA::ULong ids_len;
        ids_len <<= pe;

        for (CORBA::ULong ii=0; ii != ids_len; ++ii) {
          CORBA::UShort compressor_id, compression_level;

          compressor_id     <<= pe;
          compression_level <<= pe;
          
          cout << "                           compressor ";

          switch (compressor_id) {
          case 0:
            cout << "NONE";
            break;
          case 1:
            cout << "GZIP";
            break;
          case 2:
            cout << "PKZIP";
            break;
          case 3:
            cout << "BZIP2";
            break;
          case 4:
            cout << "ZLIB";
            break;
          case 5:
            cout << "LZMA";
            break;
          case 6:
            cout << "LZO";
            break;
          case 7:
            cout << "RZIP";
            break;
          case 8:
            cout << "7X";
            break;
          case 9:
            cout << "XAR";
            break;
          default:
            cout << "unknown";
            break;
          }
          cout << ", level " << compression_level << endl;
        }
      }
      break;

    default:
      cout << "unknown(" << ptype << ')' << endl;
      break;
    }
  }
}

static
void
print_tag_alternate_iiop_address(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::String_var host;
  CORBA::UShort     port;

  host = e.unmarshalRawString();
  port <<= e;

  cout << "      TAG_ALTERNATE_IIOP_ADDRESS " << host << ' ' << port << endl;
}

static
void
print_tag_ssl_sec_trans(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::UShort  target_supports, target_requires, port;
  CORBA::Boolean is_visi = 0;

  cout << "      TAG_SSL_SEC_TRANS";

  try {
    switch (c.component_data.length()) {
      // Remember this is an encapsulation, so the length includes the
      // first endian octet plus the necessary paddings after it
    case 8:
      {
	// This is the standard format
	target_supports <<= e;
	target_requires <<= e;
	port <<= e;
	break;
      }
    default:
      {
	// Try visibroker propriety format
	CORBA::ULong v;
	v <<= e; target_supports = v;
	v <<= e; target_requires = v;
	port <<= e;
	is_visi = 1;
	break;
      }
    }
    cout << " port = " << port
         << " supports " << target_supports
         << " requires " << target_requires;

    if (is_visi)
      cout << " (visibroker format)";
  }
  catch (...) {
    cout << " (non-standard and unknown format)";
  }
  cout << endl;
}

static
void
print_tag_csi_sec_mech_list(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::Boolean stateful = e.unmarshalBoolean();

  CORBA::ULong mech_count;
  mech_count <<= e;

  for (CORBA::ULong mech_idx = 0; mech_idx != mech_count; ++mech_idx) {
    CORBA::UShort target_requires;

    CORBA::UShort as_target_supports, as_target_requires;
    _CORBA_Unbounded_Sequence_Octet as_client_authentication_mech;
    _CORBA_Unbounded_Sequence_Octet as_target_name;

    CORBA::UShort sas_target_supports, sas_target_requires;
    CORBA::ULong sas_privilege_authorities_len;
    _CORBA_Unbounded_Sequence<_CORBA_Unbounded_Sequence_Octet> sas_supported_naming_mechanisms;
    CORBA::ULong sas_supported_identity_types;

    // CompoundSecMech structure
    target_requires <<= e;

    IOP::TaggedComponent transport_mech;
    transport_mech <<= e;

    // as_context_mech member
    as_target_supports <<= e;
    as_target_requires <<= e;
    as_client_authentication_mech <<= e;
    as_target_name <<= e;

    // sas_context_mech member
    sas_target_supports <<= e;
    sas_target_requires <<= e;
    sas_privilege_authorities_len <<= e;
    for (CORBA::ULong pi = 0; pi != sas_privilege_authorities_len; ++pi) {
      CORBA::ULong syntax;
      _CORBA_Unbounded_Sequence_Octet name;
      
      syntax <<= e;
      name   <<= e;
    }
    sas_supported_naming_mechanisms <<= e;
    sas_supported_identity_types <<= e;

    if (as_target_requires  == 0 &&
	sas_target_requires == 0 &&
	transport_mech.tag  == IOP::TAG_TLS_SEC_TRANS) {

      // No higher-level requirements and a TLS transport tag -- we
      // can support this component.
      CORBA::UShort tls_target_supports, tls_target_requires;
      CORBA::ULong addresses_len;

      cdrEncapsulationStream tls_e(transport_mech.component_data, 1);
      
      tls_target_supports <<= tls_e;
      tls_target_requires <<= tls_e;
      addresses_len <<= tls_e;

      for (CORBA::ULong ai = 0; ai != addresses_len; ++ai) {
	IIOP::Address ssladdr;

	ssladdr.host = tls_e.unmarshalRawString();
	ssladdr.port <<= tls_e;

        CORBA::String_var addr = omniURI::buildURI("", ssladdr.host,
                                                   ssladdr.port);
        if (ai == 0)
          cout << "      TAG_CSI_SEC_MECH_LIST endpoints ";
        else
          cout << "                                      ";

        cout << addr << endl;
      }
      return;
    }
  }
  cout << "      TAG_CSI_SEC_MECH_LIST (no usable endpoints)" << endl;
}

static
void
print_tag_omniorb_bidir(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::String_var sendfrom = e.unmarshalRawString();
  
  cout << "      TAG_OMNIORB_BIDIR " << sendfrom << endl;
}

static
void
print_tag_omniorb_unix_trans(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::String_var host     = e.unmarshalRawString();
  CORBA::String_var filename = e.unmarshalRawString();

  cout << "      TAG_OMNIORB_UNIX_TRANS " << host << ' '  << filename << endl;
}

static
void
print_tag_omniorb_persistent_id(const IOP::TaggedComponent& c)
{
  cout << "      TAG_OMNIORB_PERSISTENT_ID ";
  print_key(c.component_data, 1);
  cout << endl;
}

static
void
print_tag_omniorb_restricted_connection(const IOP::TaggedComponent& c)
{
  cdrEncapsulationStream e(c.component_data, 1);

  CORBA::Octet  version;
  CORBA::UShort flags;
  CORBA::ULong  connection_id, max_connections, max_threads;

  version           = e.unmarshalOctet();
  flags           <<= e;
  connection_id   <<= e;
  max_connections <<= e;
  max_threads     <<= e;

  cout << "      TAG_OMNIORB_RESTRICTED_CONNECTION version         "
       << (unsigned long)version << endl;

  cout << "                                        flags          ";
  if (flags & 0x0001) cout << " data_batch";
  if (flags & 0x0002) cout << " permit_interleaved";
  if (flags & 0x0004) cout << " hold_open";
  if (flags == 0)     cout << " (none)";
  cout << endl;

  cout << "                                        connection_id   "
       << connection_id << endl;

  cout << "                                        max_connections "
       << max_connections << endl;

  cout << "                                        max_threads     "
       << max_threads << endl;
}


static
void
print_tagged_components(IOP::MultipleComponentProfile& components)
{
  CORBA::ULong total = components.length();

  for (CORBA::ULong index=0; index < total; index++) {
    IOP::TaggedComponent& c = components[index];

    try {
      switch (c.tag) {
      case 0: // IOP::TAG_ORB_TYPE
        print_tag_orb_type(c);
        break;

      case 1: // IOP::TAG_CODE_SETS
        print_tag_code_sets(c);
        break;

      case 2: // IOP::TAG_POLICIES
        print_tag_policies(c);
        break;

      case 3: // IOP:TAG_ALTERNATE_IIOP_ADDRESS
        print_tag_alternate_iiop_address(c);
        break;

      case 20: // IOP::TAG_SSL_SEC_TRANS
        print_tag_ssl_sec_trans(c);
        break;

      case 33: // IOP::TAG_CSI_SEC_MECH_LIST
        print_tag_csi_sec_mech_list(c);
        break;

      case 0x41545401: // TAG_OMNIORB_BIDIR
        print_tag_omniorb_bidir(c);
        break;

      case 0x41545402: // TAG_OMNIORB_UNIX_TRANS
        print_tag_omniorb_unix_trans(c);
        break;

      case 0x41545403: // TAG_OMNIORB_PERSISTENT_ID
        print_tag_omniorb_persistent_id(c);
        break;

      case 0x41545404: // TAG_OMNIORB_RESTRICTED_CONNECTION
        print_tag_omniorb_restricted_connection(c);
        break;

      default:
        cout << "      Unknown component tag " << c.tag << endl;
      }
    }
    catch (CORBA::MARSHAL& ex) {
      cout << "            Broken component with tag " << c.tag << endl;
    }
  }
}

static
void
toIOR(const char* iorstr,IOP::IOR& ior)
{
  size_t s = (iorstr ? strlen(iorstr) : 0);
  if (s<4)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  const char *p = iorstr;
  if (p[0] != 'I' ||
      p[1] != 'O' ||
      p[2] != 'R' ||
      p[3] != ':')
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  cdrMemoryStream buf((CORBA::ULong)s,0);

  for (int i=0; i<(int)s; i++) {
    int j = i*2;
    CORBA::Octet v;
    
    if (p[j] >= '0' && p[j] <= '9') {
      v = ((p[j] - '0') << 4);
    }
    else if (p[j] >= 'a' && p[j] <= 'f') {
      v = ((p[j] - 'a' + 10) << 4);
    }
    else if (p[j] >= 'A' && p[j] <= 'F') {
      v = ((p[j] - 'A' + 10) << 4);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    if (p[j+1] >= '0' && p[j+1] <= '9') {
      v += (p[j+1] - '0');
    }
    else if (p[j+1] >= 'a' && p[j+1] <= 'f') {
      v += (p[j+1] - 'a' + 10);
    }
    else if (p[j+1] >= 'A' && p[j+1] <= 'F') {
      v += (p[j+1] - 'A' + 10);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    buf.marshalOctet(v);
  }

  buf.rewindInputPtr();
  CORBA::Boolean b = buf.unmarshalBoolean();
  buf.setByteSwapFlag(b);

  ior.type_id = IOP::IOR::unmarshaltype_id(buf);
  ior.profiles <<= buf;
}


int main(int argc, char* argv[])
{
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }


  // Get options:

  int c;
  int hexflag = 0;
  int omniflag = 0;

  while((c = getopt(argc,argv,"xo")) != -1) {
    switch(c) {
    case 'x':
      hexflag = 1;
      break;
    case 'o':
      omniflag = 1;
      break;
    case '?':
    case 'h':
      {
	usage(argv[0]);
	return 1;
      }
    }
  }

  if (optind >= argc) {
    usage(argv[0]);
    return 1;
  }


  const char* str_ior = argv[optind];

  CORBA::ORB_var orb;
  argc = 0;
  orb = CORBA::ORB_init(argc,0,"omniORB4");

  IOP::IOR ior;

  try {

    toIOR(str_ior, ior);

    if (ior.profiles.length() == 0 && strlen(ior.type_id) == 0) {
      cout << "IOR is a nil object reference." << endl;
    }
    else {
      cout << "Type ID: \"" << (const char*) ior.type_id << "\"" << endl;
      cout << "Profiles:" << endl;

      for (unsigned long count=0; count < ior.profiles.length(); count++) {

	cout << count+1 << ". ";

	if (ior.profiles[count].tag == IOP::TAG_INTERNET_IOP) {

	  IIOP::ProfileBody pBody;
	  IIOP::unmarshalProfile(ior.profiles[count],pBody);

	  cout << "IIOP " << (int) pBody.version.major << "."
	       << (int) pBody.version.minor << " ";
	  cout << (const char*) pBody.address.host 
	       << " " << pBody.address.port << " ";

	  unsigned long j;

	  if (omniflag)
	    print_omni_key(pBody.object_key, hexflag);
	  else
	    print_key(pBody.object_key, hexflag);

	  print_tagged_components(pBody.components);

	  cout << endl;
	}
	else if (ior.profiles[count].tag == IOP::TAG_MULTIPLE_COMPONENTS) {
	  
	  cout << "Multiple Component Profile ";
	  IIOP::ProfileBody pBody;
	  IIOP::unmarshalMultiComponentProfile(ior.profiles[count],
					       pBody.components);
	  print_tagged_components(pBody.components);

	  cout << endl;
	  
	}
	else {
	  cout << "Unrecognised profile tag: 0x"
	       << hex << (unsigned)(ior.profiles[count].tag) << dec
	       << endl;
	}
      }
    }
  }
  catch(CORBA::MARSHAL& ex) {
    cerr << "\nInvalid stringified IOR supplied." << endl;
    const char* ms = ex.NP_minorString();
    if (ms)
      cerr << "(CORBA::MARSHAL: minor = " << ms << ")" << endl;
    else
      cerr << "(CORBA::MARSHAL: minor = 0x" << hex << ex.minor() << ")"
	   << endl;
    return 1;
  }
  catch(...) {
    cerr << "\nException while processing stringified IOR." << endl;
    return 1;
  }

  orb->destroy();
  return 0;
}
