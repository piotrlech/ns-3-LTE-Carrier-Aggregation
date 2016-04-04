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

#ifndef LTE_ENB_COMPONENT_CARRIER_MANAGER_H
#define LTE_ENB_COMPONENT_CARRIER_MANAGER_H

#include <ns3/object.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/lte-ccm-rrc-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/lte-ul-ccm-mac-sap.h>
#include <map>
#include <vector>

namespace ns3 {


class LteCcmRrcSapUser;
class LteCcmRrcSapSapProvider;

class LteMacSapUser;
class LteMacSapProvider;

class LteEnbCmacSapProvider;

class LteUlCcmMAcSapProvider;


/**
 * \brief The abstract base class of a Component Carrier Manager algorithm for Enb that operates using
 *        the ComponentCarrier Management SAP interface.
 *
 * Component Carrier Manager algorithm receives measurement reports from an eNode RRC instance
 * and tells the eNodeB RRC instance when e.g. turn on a Component Carrier.
 *
 * This class is an abstract class intended to be inherited by subclasses that
 * implement its virtual methods. By inheriting from this abstract class, the
 * subclasses gain the benefits of being compatible with the LteEnbNetDevice
 * class, being accessible using namespace-based access through ns-3 Config
 * subsystem, and being installed and configured by LteHelper class (see
 * LteHelper::SetComponentCarrierAlgorithmType and
 * LteHelper::SetComponentCarrierAlgorithmAttribute methods).
 *
 * The communication with the eNodeB RRC instance is done through the *ComponentCarrier
 * Management SAP* interface. The ComponentCarrier algorithm instance corresponds to the
 * "provider" part of this interface, while the eNodeB RRC instance takes the
 * role of the "user" part. The following code skeleton establishes the
 * connection between both instances:
 *
 *     Ptr<LteEnbRrc> u = ...;
 *     Ptr<LteEnbComponentCarrierManager> p = ...;
 *     u->SetLteComponentCarrierManagementSapProvider (p->GetLteComponentCarrierManagementSapProvider ());
 *     p->SetLteComponentCarrierManagementSapUser (u->GetLteComponentCarrierManagementSapUser ());
 *
 * However, user rarely needs to use the above code, since it has already been
 * taken care by LteHelper::InstallEnbDevice.
 *
 * \sa LteComponentCarrierManagementSapProvider, LteComponentCarrierManagementSapUser
 */
class LteEnbComponentCarrierManager : public Object
{
public:
  LteEnbComponentCarrierManager ();
  virtual ~LteEnbComponentCarrierManager ();


  // inherited from Object
  static TypeId GetTypeId ();

  /**
   * \brief Set the "user" part of the ComponentCarrier Management SAP interface that
   *        this ComponentCarrier algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an LteEnbRrc instance
   */
  virtual void SetLteCcmRrcSapUser (LteCcmRrcSapUser* s) = 0;

  /**
   * \brief Set the "user" part of the ComponentCarrier Management SAP interface that
   *        this ComponentCarrier algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an LteEnbRrc instance
   */
  virtual void SetLteMacSapUser (LteMacSapUser* s) = 0;

  /**
   * \brief Export the "provider" part of the ComponentCarrier Management SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an LteEnbRlc instance
   */
  virtual LteCcmRrcSapProvider* GetLteCcmRrcSapProvider () = 0;


  // virtual void SetLteUlCcmMacSapProvider (LteUlCcmMacSapProvider* s) = 0;


  virtual LteUlCcmMacSapUser* GetLteUlCcmMacSapUser () = 0;

  /**
   * \brief Export the "provider" part of the ComponentCarrier Management SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an LteEnbRlc instance
   */
  virtual LteMacSapProvider* GetLteMacSapProvider () = 0;

  bool SetComponentCarrierMacSapProviders (uint16_t componentCarrierId, LteMacSapProvider* sap);

  bool SetUlCcmMacSapProviders (uint8_t componentCarrierId, LteUlCcmMacSapProvider* sap);

  void SetComponentCarrierNumber (uint16_t no);

  // this map is initialized in LteHelper when the Component Carrier Manager is initialized
  // User case: 
  // it=ComponentCarrierMap.begin ();
  // lteEnbComponentCarrierManager.m_noOfComponentCarrier = m_noOfComponentCarrier;
  // lteEnbComponentCarrierManager->m_CcMacSapProvider.insert(std::pair<it->first, it->second->GetMac ()->GetLteMacSapProvider () >; for each ComponentCarrier
  // componentCarrierId LteMacSapProvider of the Mac Layer 
  std::map <uint16_t, LteMacSapProvider*> m_CcMacSapProvider;

  std::map< uint8_t,LteUlCcmMacSapProvider*> m_ulCcmMacSapProviderMap;

  

protected:

  // inherited from Object
  virtual void DoDispose ();

  // COMPONENT CARRIER MANAGEMENT SAP PROVIDER IMPLEMENTATION
  /**
   * \brief Implementation of LteComponentCarrierManagementSapProvider::ReportUeMeas.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param measResults a single report of one measurement identity
   */
  virtual void DoReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults) = 0;

protected:
  //            rnti,             lcid, SAP of the RLC instance
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> > m_ueAttached;
  // rnit, lcid, LcConfiguration from 'RLC' prospective (RLC see only one LC 
  // while Component Carrier Manager see many LC as the number of Component Carrier)
  std::map <uint16_t, std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> > m_rlcLcInstantiated;
  // rnti, number of component carrier enabled;
  std::map <uint16_t, uint8_t> m_enabledComponentCarrier;
  // rnti, state 
  std::map <uint16_t, uint8_t> m_ueState;
  // Number of maximum component Carrier in the simulations
  uint16_t m_noOfComponentCarriers;

  LteUlCcmMacSapProvider* m_ulCcmMacSapProvider;
  LteUlCcmMacSapUser* m_ulCcmMacSapUser;
 
  // here the structure to split the traffic from one LC to multiple
  // is not implemented because is algorithm depended
  

}; // end of class LteEnbComponentCarrierManager


} // end of namespace ns3


#endif /* LTE_ENB_COMPONENT_CARRIER_MANAGER_H */
