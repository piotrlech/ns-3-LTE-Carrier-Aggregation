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

#ifndef NO_OP_COMPONENT_CARRIER_MANAGER_H
#define NO_OP_COMPONENT_CARRIER_MANAGER_H

#include <ns3/lte-enb-component-carrier-manager.h>
#include <ns3/lte-ccm-rrc-sap.h>
#include <ns3/lte-rrc-sap.h>
#include <map>

namespace ns3 {
  class UeManager;
  class LteCcmRrcSapProvider;

/**
 * \brief Component carrier selection algorithm implementation which simply does nothing.
 *
 * Selecting this Component carrier selection algorithm is equivalent to disabling automatic
 * triggering of Component carrier selection. This is the default choice.
 *
 * To enable automatic Component carrier selection, please select another Component carrier selection algorithm, i.e.,
 * another child class of LteCcsAlgorithm.
 */
class NoOpComponentCarrierManager : public LteEnbComponentCarrierManager
{
public:
  /// Creates a No-op CCS algorithm instance.
  NoOpComponentCarrierManager ();

  virtual ~NoOpComponentCarrierManager ();


  // inherited from Object
  static TypeId GetTypeId ();

  // inherited from LteEnbComponentCarrierManager
  virtual void SetLteCcmRrcSapUser (LteCcmRrcSapUser* s);
  virtual LteCcmRrcSapProvider* GetLteCcmRrcSapProvider ();

  virtual LteMacSapProvider* GetLteMacSapProvider ();
  virtual void SetLteMacSapUser (LteMacSapUser* s);

  // virtual void SetLteUlCcmMacSapProvider (LteUlCcmMacSapProvider* s);
  virtual LteUlCcmMacSapUser* GetLteUlCcmMacSapUser ();

  friend class EnbMacMemberLteMacSapProvider<NoOpComponentCarrierManager>;
  // let the forwarder class access the protected and private members
  friend class MemberLteCcmRrcSapProvider<NoOpComponentCarrierManager>;
  friend class MemberLteCcmRrcSapUser<NoOpComponentCarrierManager>;

  friend class MemberLteUlCcmMacSapUser<NoOpComponentCarrierManager>;
  
  friend class NoOpEnbCcmMacSapUser;

protected:
  // inherited from Object
  virtual void DoInitialize ();
  virtual void DoDispose ();

  // inherited from LteCcsAlgorithm as a Component Carrier Management SAP implementation
  void DoReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults);


private:

  // forwarded from LteMacSapProvider
  void DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params);
  void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params);

  // forwarded from LteMacSapUser
  void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  void DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);
  void DoNotifyHarqDeliveryFailure ();

  // forwarded from LteCcmRrcSapProvider
  void DoAddUe (uint16_t rnti, uint8_t state);
  void DoRemoveUe (uint16_t rnti);
  std::vector<LteCcmRrcSapProvider::LcsConfig> DoSetupDataRadioBearer (EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, LteMacSapUser* msu);
  std::vector<uint16_t> DoReleaseDataRadioBearer (uint16_t rnti, uint8_t lcid);
  LteMacSapUser* DoConfigureSignalBearer(LteEnbCmacSapProvider::LcInfo lcinfo,  LteMacSapUser* msu);

    // CCM Ul SAP MAC-CCM USER IMPLEMENTATION

  void DoUlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId);

  /// Interface to the eNodeB RRC instance.
  LteCcmRrcSapUser* m_ccmRrcSapUser;
  /// Receive API calls from the eNodeB RRC instance.
  LteCcmRrcSapProvider* m_ccmRrcSapProvider;

  ///Interface to the eNodeB RLC instance.
  LteMacSapUser* m_ccmMacSapUser;
  ///Receive API calls from the eNodeB RLC instance.
  LteMacSapProvider* m_ccmMacSapProvider;

}; // end of class NoOpComponentCarrierManager


} // end of namespace ns3


#endif /* NO_OP_COMPONENT_CARRIER_MANAGER_H */
