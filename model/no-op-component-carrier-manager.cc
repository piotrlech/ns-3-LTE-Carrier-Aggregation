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

#include "no-op-component-carrier-manager.h"
#include <ns3/log.h>
//#include <ns3/lte-mac-sap.h>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("NoOpComponentCarrierManager");

  NS_OBJECT_ENSURE_REGISTERED (NoOpComponentCarrierManager);
  
///////////////////////////////////////////////////////////
// MAC SAP USER SAP forwarders
/////////////// ////////////////////////////////////////////
class NoOpEnbCcmMacSapUser : public LteMacSapUser
{ 
public:
 NoOpEnbCcmMacSapUser  (NoOpComponentCarrierManager* mac);

  // inherited from LteMacSapUser
  virtual void NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  virtual void ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);
  virtual void NotifyHarqDeliveryFailure ();


private:
  NoOpComponentCarrierManager* m_owner;
};

NoOpEnbCcmMacSapUser::NoOpEnbCcmMacSapUser (NoOpComponentCarrierManager* mac)
  : m_owner (mac)
{
}

void
NoOpEnbCcmMacSapUser::NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  m_owner->DoNotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);
}


void
NoOpEnbCcmMacSapUser::ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  m_owner->DoReceivePdu (p, rnti, lcid);
}

void
NoOpEnbCcmMacSapUser::NotifyHarqDeliveryFailure ()
{
  m_owner->DoNotifyHarqDeliveryFailure ();
}




NoOpComponentCarrierManager::NoOpComponentCarrierManager ()
   : m_ccmRrcSapUser (0)
  {
    NS_LOG_FUNCTION (this);
    m_ccmRrcSapProvider = new MemberLteCcmRrcSapProvider<NoOpComponentCarrierManager> (this);
    m_ulCcmMacSapUser = new MemberLteUlCcmMacSapUser<NoOpComponentCarrierManager> (this);
    m_ccmMacSapUser = new NoOpEnbCcmMacSapUser (this);
    m_ccmMacSapProvider = new EnbMacMemberLteMacSapProvider <NoOpComponentCarrierManager> (this);
  }


  NoOpComponentCarrierManager::~NoOpComponentCarrierManager ()
  {
    NS_LOG_FUNCTION (this);
  }


  void
  NoOpComponentCarrierManager::DoDispose ()
  {
    NS_LOG_FUNCTION (this);
    delete m_ccmRrcSapProvider;
    delete m_ulCcmMacSapUser;
  }


  TypeId
  NoOpComponentCarrierManager::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::NoOpComponentCarrierManager")
      .SetParent<LteEnbComponentCarrierManager> ()
      .SetGroupName("Lte")
      .AddConstructor<NoOpComponentCarrierManager> ()
      ;
    return tid;
  }

  void
  NoOpComponentCarrierManager::SetLteCcmRrcSapUser (LteCcmRrcSapUser* s)
  {
    NS_LOG_FUNCTION (this << s);
    m_ccmRrcSapUser = s;
  }

  void
  NoOpComponentCarrierManager::SetLteMacSapUser (LteMacSapUser* s)
  {
    NS_LOG_FUNCTION (this << s);
    m_ccmMacSapUser = s;
  }

  LteCcmRrcSapProvider*
  NoOpComponentCarrierManager::GetLteCcmRrcSapProvider ()
  {
    NS_LOG_FUNCTION (this);
    return m_ccmRrcSapProvider;
  }

  LteMacSapProvider*
  NoOpComponentCarrierManager::GetLteMacSapProvider ()
  {
    NS_LOG_FUNCTION (this);
    return m_ccmMacSapProvider;
  }

  LteUlCcmMacSapUser*
  NoOpComponentCarrierManager::GetLteUlCcmMacSapUser ()
  {
    NS_LOG_FUNCTION (this);
    return m_ulCcmMacSapUser;
  }

  // void
  // NoOpComponentCarrierManager::SetLteUlCcmMacSapProvider (LteUlCcmMacSapProvider* s)
  // {
  //   NS_LOG_FUNCTION (this << s);
  //   m_ulCcmMacSapProvider = s;
  // }

  void
  NoOpComponentCarrierManager::DoInitialize ()
  {
    NS_LOG_FUNCTION (this);
    LteEnbComponentCarrierManager::DoInitialize ();
  }


  void
  NoOpComponentCarrierManager::DoReportUeMeas (uint16_t rnti,
                                               LteRrcSap::MeasResults measResults)
  {
    NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
  }

  // ////////////////////////////////////////////
  // MAC SAP
  // ////////////////////////////////////////////


  void
  NoOpComponentCarrierManager::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
  {
    NS_LOG_FUNCTION (this);
    std::map <uint16_t, LteMacSapProvider*>::iterator it =  m_CcMacSapProvider.find (params.componentCarrierId);
    NS_ASSERT_MSG (it != m_CcMacSapProvider.end (), "could not find Sap for ComponentCarrier " << params.componentCarrierId);
    // with this algorithm all traffic is on Primary Carrier
    it->second->TransmitPdu (params);
  }

  void
  NoOpComponentCarrierManager::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
  {
    NS_LOG_FUNCTION (this);
    std::map <uint16_t, LteMacSapProvider*>::iterator it =  m_CcMacSapProvider.find (0);
    NS_ASSERT_MSG (it != m_CcMacSapProvider.end (), "could not find Sap for ComponentCarrier ");
    it->second->ReportBufferStatus (params);
  }

  void
  NoOpComponentCarrierManager::DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
  {
    NS_LOG_FUNCTION (this);
    std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (rnti);
    NS_ASSERT_MSG (rntiIt != m_ueAttached.end (), "could not find RNTI" << rnti);
    std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcid);
    NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID " << (uint16_t) lcid);
    NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid << " layer= " << layer);
    (*lcidIt).second->NotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);

  }

  void
  NoOpComponentCarrierManager::DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
  {
    NS_LOG_FUNCTION (this);
    std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (rnti);
    NS_ASSERT_MSG (rntiIt != m_ueAttached.end (), "could not find RNTI" << rnti);
    std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcid);
    if (lcidIt != rntiIt->second.end ())
      {
        (*lcidIt).second->ReceivePdu (p, rnti, lcid);
      }
  }
  
void 
NoOpComponentCarrierManager::DoNotifyHarqDeliveryFailure ()
{
 NS_LOG_FUNCTION (this);
}

  void
  NoOpComponentCarrierManager::DoAddUe (uint16_t rnti, uint8_t state)
  {
    NS_LOG_FUNCTION (this << rnti << (uint16_t) state);
    std::map<uint16_t, uint8_t>::iterator stateIt;
    std::map<uint16_t, uint8_t>::iterator eccIt; // m_enabledComponentCarrier iterator
    stateIt = m_ueState.find (rnti);
    if (stateIt == m_ueState.end ())
      {
//        NS_ASSERT_MSG ((stateIt == m_ueState.end () && state == 3), " ERROR: Ue was not indexed and current state is CONNECTED_NORMALLY" << (uint16_t) state);
        NS_LOG_DEBUG (this << " UE " << rnti << " was not found, now it is added in the map");
        m_ueState.insert (std::pair<uint16_t, uint8_t> (rnti, state));
        eccIt = m_enabledComponentCarrier.find (rnti);
        //if ((state == 7 || state == 0) && eccIt == m_enabledComponentCarrier.end ())
        if (eccIt == m_enabledComponentCarrier.end ())
          {
            // the Primary carrier (PC) is enabled by default
            // on the PC the SRB0 and SRB1 are enabled when the Ue is connected
            // these are hard-coded and the configuration not pass through the 
            // Component Carrier Manager which is responsible of configure
            // only DataRadioBearer on the different Component Carrier
            m_enabledComponentCarrier.insert (std::pair<uint16_t, uint8_t> (rnti, 1));
          }
        else
          {
            NS_FATAL_ERROR (this << " Ue " << rnti << " had Component Carrier enabled before join the network" << (uint16_t) state);
          }
        // preparing the rnti,lcid,LteMacSapUser map
        std::map<uint8_t, LteMacSapUser*> empty;
        std::pair <std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator, bool> 
          ret = m_ueAttached.insert (std::pair <uint16_t,  std::map<uint8_t, LteMacSapUser*> > 
                                     (rnti, empty));
        NS_LOG_DEBUG (this << "AddUe: UE Pointer LteMacSapUser Map " << rnti << " added " << (uint16_t) ret.second);
        NS_ASSERT_MSG (ret.second, "element already present, RNTI already existed");
 
        //add new rnti in the map
        std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> emptyA;
        std::pair <std::map <uint16_t, std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> >::iterator, bool> 
          retA = m_rlcLcInstantiated.insert (std::pair <uint16_t,  std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> > 
                                            (rnti, emptyA));
        NS_ASSERT_MSG (retA.second, "element already present, RNTI already existed");
        NS_LOG_DEBUG (this << "AddUe: UE " << rnti << " added " << (uint16_t) retA.second);

      }
    else
      {
        NS_LOG_DEBUG (this << " UE " << rnti << "found, updating the state from " << (uint16_t) stateIt->second << " to " << (uint16_t) state);
        stateIt->second = state;
      }
  
  
  }

  void
  NoOpComponentCarrierManager::DoRemoveUe (uint16_t rnti)
  {
    NS_LOG_FUNCTION (this);
    std::map<uint16_t, uint8_t>::iterator stateIt;
    std::map<uint16_t, uint8_t>::iterator eccIt; // m_enabledComponentCarrier iterator
    stateIt = m_ueState.find (rnti);
    eccIt = m_enabledComponentCarrier.find (rnti);
    NS_ASSERT_MSG (stateIt != m_ueState.end (), "request to remove UE info with unknown rnti ");  
    NS_ASSERT_MSG (eccIt != m_enabledComponentCarrier.end (), "request to remove UE info with unknown rnti ");

  }

  std::vector<LteCcmRrcSapProvider::LcsConfig>
  NoOpComponentCarrierManager::DoSetupDataRadioBearer (EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, LteMacSapUser *msu)
  {
    NS_LOG_FUNCTION (this << rnti);
    std::map<uint16_t, uint8_t>::iterator eccIt; // m_enabledComponentCarrier iterator
    eccIt = m_enabledComponentCarrier.find (rnti);
    NS_ASSERT_MSG (eccIt != m_enabledComponentCarrier.end (), "SetupDataRadioBearer on unknown rnti ");
    std::vector<LteCcmRrcSapProvider::LcsConfig> res;
    LteCcmRrcSapProvider::LcsConfig entry;
    LteEnbCmacSapProvider::LcInfo lcinfo;
    // NS_LOG_DEBUG (this << " componentCarrierEnabled " << (uint16_t) eccIt->second);
    for (uint16_t ncc = 0; ncc < eccIt->second; ncc++)
      {
        // NS_LOG_DEBUG (this << " res size " << (uint16_t) res.size ());
        LteEnbCmacSapProvider::LcInfo lci;
        lci.rnti = rnti;
        lci.lcId = lcid;
        lci.lcGroup = lcGroup;
        lci.qci = bearer.qci;
        if (ncc == 0)
          {
            lci.isGbr = bearer.IsGbr ();
            lci.mbrUl = bearer.gbrQosInfo.mbrUl;
            lci.mbrDl = bearer.gbrQosInfo.mbrDl;
            lci.gbrUl = bearer.gbrQosInfo.gbrUl;
            lci.gbrDl = bearer.gbrQosInfo.gbrDl;
          }
        else
          {
            lci.isGbr = 0;
            lci.mbrUl = 0;
            lci.mbrDl = 0;
            lci.gbrUl = 0;
            lci.gbrDl = 0;
          } // data flows only on PC
        NS_LOG_DEBUG (this << " RNTI " << lci.rnti << "Lcid " << (uint16_t) lci.lcId << " lcGroup " << (uint16_t) lci.lcGroup);
        entry.componentCarrierId = ncc;
        entry.lc = lci;
        entry.msu = m_ccmMacSapUser;
        res.push_back (entry);
      } // end for


    // preparing the rnti,lcid,LcInfo map
    std::map <uint16_t, std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> >::iterator rntiIter = m_rlcLcInstantiated.find (rnti);
    rntiIter = m_rlcLcInstantiated.begin ();
    // while (rntiIter != m_rlcLcInstantiated.end ())
    //   {
    //     std::cout << " RNTI in maps " << rntiIter->first << std::endl;
    //     ++rntiIter;
    //   }
    // if (rntiIt == m_rlcLcInstantiated.end ())
    //   {
    //     //add new rnti in the map
    //     std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> empty;
    //     std::pair <std::map <uint16_t, std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> >::iterator, bool> 
    //       ret = m_rlcLcInstantiated.insert (std::pair <uint16_t,  std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> > 
    //                                         (rnti, empty));
    //     NS_LOG_DEBUG (this << " UE " << rnti << " added " << (uint16_t) ret.second);
    //   }

    std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator sapIt =  m_ueAttached.find (rnti);
    NS_ASSERT_MSG (sapIt != m_ueAttached.end (), "RNTI not found");
    rntiIter = m_rlcLcInstantiated.find (rnti);
    std::map<uint8_t, LteEnbCmacSapProvider::LcInfo>::iterator lcidIt = rntiIter->second.find (lcid);
    //std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = sapIt->second.find (lcinfo.lcId);
    rntiIter = m_rlcLcInstantiated.find (rnti);
    NS_ASSERT_MSG (rntiIter != m_rlcLcInstantiated.end (), "RNTI not found");
    if (lcidIt == rntiIter->second.end ())
      {
        lcinfo.rnti = rnti;
        lcinfo.lcId = lcid;
        lcinfo.lcGroup = lcGroup;
        lcinfo.qci = bearer.qci;
        lcinfo.isGbr = bearer.IsGbr ();
        lcinfo.mbrUl = bearer.gbrQosInfo.mbrUl;
        lcinfo.mbrDl = bearer.gbrQosInfo.mbrDl;
        lcinfo.gbrUl = bearer.gbrQosInfo.gbrUl;
        lcinfo.gbrDl = bearer.gbrQosInfo.gbrDl;
        rntiIter->second.insert (std::pair<uint8_t, LteEnbCmacSapProvider::LcInfo> (lcinfo.lcId, lcinfo));
        sapIt->second.insert (std::pair<uint8_t, LteMacSapUser*> (lcinfo.lcId, msu));
      }
    else
      {
        NS_LOG_ERROR ("LC already exists");
      }     
    return res;
 
  }

  std::vector<uint16_t>
  NoOpComponentCarrierManager::DoReleaseDataRadioBearer (uint16_t rnti, uint8_t lcid)
  {
    NS_LOG_FUNCTION (this);
    // here we receive directly the rnti and the lcid, instead of only drbid
    // drbid are mapped as drbid = lcid + 2
    std::map<uint16_t, uint8_t>::iterator eccIt; // m_enabledComponentCarrier iterator
    eccIt= m_enabledComponentCarrier.find (rnti);
    NS_ASSERT_MSG (eccIt != m_enabledComponentCarrier.end (), "request to Release Data Radio Bearer on Ue without Component Carrier Enabled");
    std::map <uint16_t, std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> >::iterator lcsIt;
    lcsIt = m_rlcLcInstantiated.find (rnti);
    NS_ASSERT_MSG (lcsIt != m_rlcLcInstantiated.end (), "request to Release Data Radio Bearer on Ue without Logical Channels enabled");
    std::map<uint8_t, LteEnbCmacSapProvider::LcInfo>::iterator lcIt;
    NS_LOG_DEBUG (this << " remove lcid " << (uint16_t) lcid << " for rnti " << rnti);
    lcIt = lcsIt->second.find (lcid);
    NS_ASSERT_MSG (lcIt != lcsIt->second.end (), " Logical Channel not found");
    std::vector<uint16_t> res;
    for (uint16_t i = 0; i < eccIt->second; i++)
      {
        res.insert (res.end (), i);
      }
    //Find user based on rnti and then erase lcid stored against the same
    std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (rnti);
    rntiIt->second.erase (lcid);
    std::map <uint16_t, std::map<uint8_t, LteEnbCmacSapProvider::LcInfo> >::iterator rlcInstancesIt = m_rlcLcInstantiated.find (rnti);
    std::map<uint8_t, LteEnbCmacSapProvider::LcInfo>::iterator rclLcIt;
    lcIt = rlcInstancesIt->second.find (lcid);
    NS_ASSERT_MSG (lcIt != lcsIt->second.end (), " Erasing: Logical Channel not found");
    lcsIt->second.erase (lcid);      
    return res;
  }

  LteMacSapUser* 
  NoOpComponentCarrierManager::DoConfigureSignalBearer(LteEnbCmacSapProvider::LcInfo lcinfo,  LteMacSapUser* msu)
  {
    NS_LOG_FUNCTION (this);
    std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator itSapUserAtCcm;
    itSapUserAtCcm = m_ueAttached.find (lcinfo.rnti);
    NS_ASSERT_MSG (itSapUserAtCcm != m_ueAttached.end (), "request to Add a SignalBearer to unknown rnti");
    std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (lcinfo.rnti);
    NS_ASSERT_MSG (rntiIt != m_ueAttached.end (), "RNTI not found");
    std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcinfo.lcId);
    if (lcidIt == rntiIt->second.end ())
      {
        rntiIt->second.insert (std::pair<uint8_t, LteMacSapUser*> (lcinfo.lcId, msu));
      }
    else
      {
        NS_LOG_ERROR ("LC already exists");
      }
 
    return m_ccmMacSapUser;
  }

  void
  NoOpComponentCarrierManager::DoUlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT_MSG (componentCarrierId == 0, "Received BSR from a ComponentCarrier not allowed, ComponentCarrierId = " << componentCarrierId);
    NS_ASSERT_MSG (bsr.m_macCeType == MacCeListElement_s::BSR, "Received a Control Message not allowed " << bsr.m_macCeType);
    MacCeListElement_s newBsr;
    if ( bsr.m_macCeType == MacCeListElement_s::BSR)
      {
        newBsr.m_rnti = bsr.m_rnti;
        newBsr.m_macCeType = bsr.m_macCeType;
        newBsr.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
        newBsr.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
        newBsr.m_macCeValue.m_bufferStatus.resize (4);
        for (uint16_t i = 0; i < 4; i++)
          {
            uint8_t bsrId = bsr.m_macCeValue.m_bufferStatus.at (i);
            uint32_t buffer = BufferSizeLevelBsr::BsrId2BufferSize (bsrId);
            // here the buffer should be divide among the different sap
            // since the buffer status report are compressed information
            // it is needed to use BsrId2BufferSize to uncompress
            // after the split over all component carriers is is needed to 
            // compress again the information to fit MacCeListEkement_s structure
            // verify how many Component Carrier are enabled per UE
            // in this simple code the BufferStatus will be notify only
            // to the primary carrier component
            bsr.m_macCeValue.m_bufferStatus.at (i) = BufferSizeLevelBsr::BufferSize2BsrId (buffer);
          }
      } 
 
    std::map< uint8_t,LteUlCcmMacSapProvider*>::iterator sapIt = m_ulCcmMacSapProviderMap.find (0);
    if (sapIt == m_ulCcmMacSapProviderMap.end ())
      {
        NS_FATAL_ERROR ("Sap not found in the UlCcmMacSapProviderMap (submap)");
      }
    else
      {
        // sapIt->second->ReportMacCeToScheduler (newBsr);
        sapIt->second->ReportMacCeToScheduler (bsr);
      } 
  } 

} // end of namespace ns3
