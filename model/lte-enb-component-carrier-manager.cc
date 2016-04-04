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
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it>
 *
 */

#include "lte-enb-component-carrier-manager.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteEnbComponentCarrierManager");

NS_OBJECT_ENSURE_REGISTERED (LteEnbComponentCarrierManager);


LteEnbComponentCarrierManager::LteEnbComponentCarrierManager ()
{
}


LteEnbComponentCarrierManager::~LteEnbComponentCarrierManager ()
{
}


TypeId
LteEnbComponentCarrierManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::LteEnbComponentCarrierManager")
    .SetParent<Object> ()
    .SetGroupName("Lte")
  ;
  return tid;
}


void
LteEnbComponentCarrierManager::DoDispose ()
{
}

bool 
LteEnbComponentCarrierManager::SetComponentCarrierMacSapProviders (uint16_t componentCarrierId, LteMacSapProvider* sap)
{
  NS_LOG_FUNCTION (this);
  bool res = false;
  std::map <uint16_t, LteMacSapProvider*>::iterator it;
  it=m_CcMacSapProvider.find (componentCarrierId);
  if (componentCarrierId > m_noOfComponentCarriers)
    {
      NS_FATAL_ERROR ("Inconsistent componentCarrierId or you didn't call SetComponentCarrierNumber before calling this method");
    }
  if (it != m_CcMacSapProvider.end ())
    {
      NS_FATAL_ERROR ("Tried to allocated an existing componentCarrierId");
    }
  else
    {
      m_CcMacSapProvider.insert (std::pair<uint16_t, LteMacSapProvider*>(componentCarrierId, sap));
      res = true;
    }
  return res;
  
}

bool 
LteEnbComponentCarrierManager::SetUlCcmMacSapProviders (uint8_t componentCarrierId, LteUlCcmMacSapProvider* sap)
{
  NS_LOG_FUNCTION (this);
  bool res = false;
  std::map< uint8_t,LteUlCcmMacSapProvider*>::iterator it =  m_ulCcmMacSapProviderMap.find (componentCarrierId);
  
  if (it == m_ulCcmMacSapProviderMap.end ())
    {
      m_ulCcmMacSapProviderMap.insert (std::pair <uint8_t,LteUlCcmMacSapProvider*> (componentCarrierId, sap));

    }
 
  res = true;
  return res;
  
}

void
LteEnbComponentCarrierManager::SetComponentCarrierNumber (uint16_t no)
{
  NS_LOG_FUNCTION (this);
  m_noOfComponentCarriers = no;
}

} // end of namespace ns3
