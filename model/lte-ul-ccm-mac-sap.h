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

#ifndef LTE_UL_CCM_MAC_SAP_H
#define LTE_UL_CCM_MAC_SAP_H

#include <ns3/lte-rrc-sap.h>
#include <ns3/eps-bearer.h>
#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/ff-mac-common.h>


namespace ns3 {
/**
 * \ingroup lte
 *
 * \brief Service Access Point (SAP) offered by the component carrier selection algorithm instance
 *        to the eNodeB RRC instance.
 *
 * This is the *Component Carrier Management SAP Provider*, i.e., the part of the SAP
 * that contains the component carrier selection algorithm methods called by the eNodeB RRC
 * instance.
 */
class LteUlCcmMacSapProvider
{
 
public:
  virtual ~LteUlCcmMacSapProvider ();

  /**
   * \brief Add the Buffer Status Report to the list.
   * \param bsr Buffer Status Report Received from the LteEnbComponentCarrierManager
   */
  virtual void ReportMacCeToScheduler (MacCeListElement_s bsr) = 0;


}; // end of class LteUlCcmMacSapProvider



class LteUlCcmMacSapUser
{
public:
  virtual ~LteUlCcmMacSapUser ();
  /**
   * \brief When the Primary Component carrier receive a buffer status report it is sent to the LteEnbComponentCarrierManager
   * \param bsr Buffer Status Report received from a Ue
   * \param componentCarrierId
   */
  virtual void UlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId) = 0;

}; // end of class LteUlCcmMacSapUser

template <class C>
class MemberLteUlCcmMacSapProvider : public LteUlCcmMacSapProvider
{
public:
  MemberLteUlCcmMacSapProvider (C* owner);

  // inherited from LteCcmRrcSapProvider
  virtual void ReportMacCeToScheduler (MacCeListElement_s bsr);

private:
  C* m_owner;
};

template <class C>
MemberLteUlCcmMacSapProvider<C>::MemberLteUlCcmMacSapProvider (C* owner)
  : m_owner (owner)
{
}
 
template <class C>
void MemberLteUlCcmMacSapProvider<C>::ReportMacCeToScheduler (MacCeListElement_s bsr)
{
  m_owner->DoReportMacCeToScheduler (bsr);
}


template <class C>
class MemberLteUlCcmMacSapUser : public LteUlCcmMacSapUser
{
public:
  MemberLteUlCcmMacSapUser (C* owner);

  // inherited from LteCcmRrcSapUser
  virtual void UlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId);


private:
  C* m_owner;
};

template <class C>
MemberLteUlCcmMacSapUser<C>::MemberLteUlCcmMacSapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
void MemberLteUlCcmMacSapUser<C>::UlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId)
{
  m_owner->DoUlReceiveMacCe (bsr, componentCarrierId);
}

  
} // end of namespace ns3


#endif /* LTE_UL_CCM_MAC_SAP_H */

