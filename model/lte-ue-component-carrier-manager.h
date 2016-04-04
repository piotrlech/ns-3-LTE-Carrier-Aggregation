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

#ifndef LTE_UE_COMPONENT_CARRIER_MANAGER_H
#define LTE_UE_COMPONENT_CARRIER_MANAGER_H

#include <ns3/object.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/lte-ue-ccm-rrc-sap.h>
#include <ns3/lte-mac-sap.h>
#include <map>
#include <vector>

namespace ns3 {


class LteUeCcmRrcSapUser;
class LteUeCcmRrcSapProvider;

class LteMacSapUser;
class LteMacSapProvider;


/**
 * \brief The abstract base class of a Component Carrier Manager algorithm for Ue that operates using
 *        the ComponentCarrier Management SAP interface.
 *
 * \sa LteComponentCarrierManagementSapProvider, LteComponentCarrierManagementSapUser
 */
class LteUeComponentCarrierManager : public Object
{
public:
  LteUeComponentCarrierManager ();
  virtual ~LteUeComponentCarrierManager ();

  // inherited from Object
  static TypeId GetTypeId ();

  /**
   * \brief Set the "user" part of the ComponentCarrier Management SAP interface that
   *        this ComponentCarrier algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an LteEnbRrc instance
   */
  virtual void SetLteCcmRrcSapUser (LteUeCcmRrcSapUser* s) = 0;

  /**
   * \brief Export the "provider" part of the ComponentCarrier Management SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an LteUeRrc instance
   */
  virtual LteUeCcmRrcSapProvider* GetLteCcmRrcSapProvider () = 0;
  
  virtual LteMacSapProvider* GetLteMacSapProvider () = 0;



  bool SetComponentCarrierMacSapProviders (uint16_t componentCarrierId, LteMacSapProvider* sap);

  void SetComponentCarrierNumber (uint16_t no);

  // componentCarrierId LteMacSapProvider of the Mac Layer 
  std::map <uint16_t, LteMacSapProvider*> m_CcMacSapProvider;
protected:

  // inherited from Object
  virtual void DoDispose ();

  // COMPONENT CARRIER MANAGEMENT SAP PROVIDER IMPLEMENTATION
  // lcid, SAP of the RLC instance
  std::map<uint8_t, LteMacSapUser*> m_lcAttached;
  // rnit, lcid, LcConfiguration from 'RLC' prospective (RLC see only one LC) 

  std::map<uint16_t, std::map<uint8_t, LteMacSapProvider*> > m_componentCarrierLcMap;
  // Number of maximum component Carrier in the simulations
  uint16_t m_noOfComponentCarriers;

  // Number of ComponentCarrier Enabled
  uint16_t m_noOfComponentCarriersEnabled;

}; // end of class LteUeComponentCarrierManager


} // end of namespace ns3


#endif /* LTE_UE_COMPONENT_CARRIER_MANAGER_H */
