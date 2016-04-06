/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Danilo Abrignani
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 */


#include "cc-helper.h"
#include <ns3/component-carrier-enb.h>
#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <iostream>
#include <ns3/uinteger.h>

#define MIN_CC 1
#define MAX_CC 2
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CcHelper");

NS_OBJECT_ENSURE_REGISTERED (CcHelper);

CcHelper::CcHelper (void)
{
  NS_LOG_FUNCTION (this);
  m_ccFactory.SetTypeId (ComponentCarrierEnb::GetTypeId ());
}

void
CcHelper::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId CcHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::CcHelper")
    .SetParent<Object> ()
    .AddConstructor<CcHelper> ()
    .AddAttribute ("NumberOfComponentCarriers",
                   "Set the number of Component Carriers to setup per eNodeB"
                   "Currently the maximum Number of Component Carriers allowed is 2",
                   UintegerValue (1),
                   MakeUintegerAccessor (&CcHelper::m_numberOfComponentCarries),
                   MakeUintegerChecker<uint16_t> (MIN_CC, MAX_CC))
    .AddAttribute ("UlFreq",
                   "Set Ul Channel [EARFCN] for the first carrier component",
                   UintegerValue (0),
                   MakeUintegerAccessor (&CcHelper::m_ulEarfcn),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("DlFreq",
                   "Set Dl Channel [EARFCN] for the first carrier component",
                   UintegerValue (0),
                   MakeUintegerAccessor (&CcHelper::m_dlEarfcn),
                   MakeUintegerChecker<uint16_t> (0))
    .AddAttribute ("DlBandwidth",
                   "Set Dl Bandwidth for the first carrier component",
                   UintegerValue (25),
                   MakeUintegerAccessor (&CcHelper::m_dlBandwidth),
                   MakeUintegerChecker<uint16_t> (0,100))
    .AddAttribute ("UlBandwidth",
                   "Set Dl Bandwidth for the first carrier component",
                   UintegerValue (25),
                   MakeUintegerAccessor (&CcHelper::m_ulBandwidth),
                   MakeUintegerChecker<uint16_t> (0,100))
  ;

  //NS_LOG_UNCOND ("CcHelper::GetTypeId::m_enbComponentCarriers=" << &CcHelper::m_numberOfComponentCarries);
  return tid;
}

CcHelper::~CcHelper (void)
{
  NS_LOG_FUNCTION (this);
}

void
CcHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void 
CcHelper::SetCcAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_ccFactory.Set (n, v);
}

Ptr<ComponentCarrierEnb>
CcHelper::DoCreateSingleCc (uint16_t ulBandwidth, uint16_t dlBandwidth, uint16_t ulEarfcn, uint16_t dlEarfcn, bool isPrimary)
{
  return CreateSingleCc (ulBandwidth, dlBandwidth, ulEarfcn, dlEarfcn, isPrimary);
}

std::map< uint8_t, Ptr<ComponentCarrierEnb> > 
CcHelper::EquallySpacedCcs ()
{
  std::map< uint8_t, Ptr<ComponentCarrierEnb> > ccmap;

  //m_numberOfComponentCarries = 2;
  //NS_LOG_UNCOND ("CcHelper::EquallySpacedCcs::m_enbComponentCarriers=" << m_numberOfComponentCarries << ", m_dlBandwidth=" << m_dlBandwidth);
  for (uint16_t i = 0; i < m_numberOfComponentCarries; i++)
    {
      bool pc =false;
      uint16_t ul = m_ulEarfcn + i * m_ulBandwidth;
      uint16_t dl = m_dlEarfcn + i * m_dlBandwidth;
      if (i == 0)
        {
          pc = true;
        }
      Ptr<ComponentCarrierEnb> cc = CreateSingleCc (m_ulBandwidth, m_dlBandwidth, ul, dl, pc);
      //NS_LOG_UNCOND("CcHelper::DoCreateSingleCc::GetPhy()=" << cc->GetPhy() << ", GetMac()=" << cc->GetMac() );
      ccmap.insert (std::pair<uint8_t,Ptr<ComponentCarrierEnb> >(i, cc));
    }

  //NS_LOG_UNCOND ("CcHelper::EquallySpacedCcs::ccmap.size=" << ccmap.size ());
  return ccmap;
}

Ptr<ComponentCarrierEnb> 
CcHelper::CreateSingleCc (uint16_t ulBandwidth, uint16_t dlBandwidth, uint16_t ulEarfcn, uint16_t dlEarfcn, bool isPrimary)
{
  Ptr <ComponentCarrierEnb> cc =  CreateObject<ComponentCarrierEnb> ();
  if ( m_ulEarfcn != 0)
    {
      cc->SetUlEarfcn (ulEarfcn);
    }
  else 
    {
      uint16_t ul = cc->GetUlEarfcn () + ulEarfcn;
      cc->SetUlEarfcn (ul);
    }
  if ( m_dlEarfcn != 0)
    {
      cc->SetDlEarfcn (dlEarfcn);
    }
  else 
    {
      uint16_t dl = cc->GetDlEarfcn () + dlEarfcn;
      cc->SetDlEarfcn (dl);
    }
  cc->SetDlBandwidth (dlBandwidth);
  cc->SetUlBandwidth (ulBandwidth);

  cc->SetAsPrimary (isPrimary);

  return cc;


}

} // namespace ns3
