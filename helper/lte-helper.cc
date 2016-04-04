/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es> (re-wrote from scratch this helper)
 *         Giuseppe Piro <g.piro@poliba.it> (parts of the PHY & channel  creation & configuration copied from the GSoC 2011 code)
 *         Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 */


#include "lte-helper.h"
#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/epc-enb-application.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/lte-ue-mac.h>
#include <ns3/lte-enb-mac.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-enb-phy.h>
#include <ns3/lte-ue-phy.h>
#include <ns3/lte-spectrum-phy.h>
#include <ns3/lte-chunk-processor.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/trace-fading-loss-model.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/lte-ffr-algorithm.h>
#include <ns3/lte-handover-algorithm.h>
#include <ns3/lte-enb-component-carrier-manager.h>
#include <ns3/lte-ue-component-carrier-manager.h>
#include <ns3/lte-anr.h>
#include <ns3/lte-rlc.h>
#include <ns3/lte-rlc-um.h>
#include <ns3/lte-rlc-am.h>
#include <ns3/epc-enb-s1-sap.h>
#include <ns3/lte-rrc-protocol-ideal.h>
#include <ns3/lte-rrc-protocol-real.h>
#include <ns3/mac-stats-calculator.h>
#include <ns3/phy-stats-calculator.h>
#include <ns3/phy-tx-stats-calculator.h>
#include <ns3/phy-rx-stats-calculator.h>
#include <ns3/epc-helper.h>
#include <iostream>
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/lte-spectrum-value-helper.h>
#include <ns3/epc-x2.h>
#include <ns3/cc-helper.h>

#include <ns3/pointer.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteHelper");

NS_OBJECT_ENSURE_REGISTERED (LteHelper);

LteHelper::LteHelper (void)
  : m_fadingStreamsAssigned (false),
    m_imsiCounter (0),
    m_cellIdCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_enbNetDeviceFactory.SetTypeId (LteEnbNetDevice::GetTypeId ());
  m_enbAntennaModelFactory.SetTypeId (IsotropicAntennaModel::GetTypeId ());
  m_ueNetDeviceFactory.SetTypeId (LteUeNetDevice::GetTypeId ());
  m_ueAntennaModelFactory.SetTypeId (IsotropicAntennaModel::GetTypeId ());
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
}

void
LteHelper::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_phyStats = CreateObject<PhyStatsCalculator> ();
  m_phyTxStats = CreateObject<PhyTxStatsCalculator> ();
  m_phyRxStats = CreateObject<PhyRxStatsCalculator> ();
  m_macStats = CreateObject<MacStatsCalculator> ();
  Object::DoInitialize ();
}


LteHelper::~LteHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId LteHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::LteHelper")
    .SetParent<Object> ()
    .AddConstructor<LteHelper> ()
    .AddAttribute ("Scheduler",
                   "The type of scheduler to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::FfMacScheduler.",
                   StringValue ("ns3::PfFfMacScheduler"),
                   MakeStringAccessor (&LteHelper::SetSchedulerType,
                                       &LteHelper::GetSchedulerType),
                   MakeStringChecker ())
    .AddAttribute ("FfrAlgorithm",
                   "The type of FFR algorithm to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::LteFfrAlgorithm.",
                   StringValue ("ns3::LteFrNoOpAlgorithm"),
                   MakeStringAccessor (&LteHelper::SetFfrAlgorithmType,
                                       &LteHelper::GetFfrAlgorithmType),
                   MakeStringChecker ())
    .AddAttribute ("HandoverAlgorithm",
                   "The type of handover algorithm to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::LteHandoverAlgorithm.",
                   StringValue ("ns3::NoOpHandoverAlgorithm"),
                   MakeStringAccessor (&LteHelper::SetHandoverAlgorithmType,
                                       &LteHelper::GetHandoverAlgorithmType),
                   MakeStringChecker ())
    .AddAttribute ("PathlossModel",
                   "The type of pathloss model to be used. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::PropagationLossModel.",
                   StringValue ("ns3::FriisPropagationLossModel"),
                   MakeStringAccessor (&LteHelper::SetPathlossModelType),
                   MakeStringChecker ())
    .AddAttribute ("FadingModel",
                   "The type of fading model to be used."
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::SpectrumPropagationLossModel."
                   "If the type is set to an empty string, no fading model is used.",
                   StringValue (""),
                   MakeStringAccessor (&LteHelper::SetFadingModel),
                   MakeStringChecker ())
    .AddAttribute ("UseIdealRrc",
                   "If true, LteRrcProtocolIdeal will be used for RRC signaling. "
                   "If false, LteRrcProtocolReal will be used.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&LteHelper::m_useIdealRrc),
                   MakeBooleanChecker ())
    .AddAttribute ("AnrEnabled",
                   "Activate or deactivate Automatic Neighbour Relation function",
                   BooleanValue (true),
                   MakeBooleanAccessor (&LteHelper::m_isAnrEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("UsePdschForCqiGeneration",
                   "If true, DL-CQI will be calculated from PDCCH as signal and PDSCH as interference "
                   "If false, DL-CQI will be calculated from PDCCH as signal and PDCCH as interference  ",
                   BooleanValue (true),
                   MakeBooleanAccessor (&LteHelper::m_usePdschForCqiGeneration),
                   MakeBooleanChecker ())
    .AddAttribute ("EnbComponentCarrierManager",
                   "The type of Component Carrier Manager to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::LteEnbComponentCarrierManager.",
                   StringValue ("ns3::NoOpComponentCarrierManager"),
                   MakeStringAccessor (&LteHelper::SetEnbComponentCarrierManagerType,
                                       &LteHelper::GetEnbComponentCarrierManagerType),
                   MakeStringChecker ())
    .AddAttribute ("UeComponentCarrierManager",
                   "The type of Component Carrier Manager to be used for Ues. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::LteUeComponentCarrierManager.",
                   StringValue ("ns3::SimpleUeComponentCarrierManager"),
                   MakeStringAccessor (&LteHelper::SetUeComponentCarrierManagerType,
                                       &LteHelper::GetUeComponentCarrierManagerType),
                   MakeStringChecker ())
    .AddAttribute ("UseCa",
                   "If true, Carrier Aggregation is enabled and a valid Component Carrier Map is expected "
                   "If false, single carrier simulation  ",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteHelper::m_useCa),
                   MakeBooleanChecker ())
    .AddAttribute ("NumberOfComponentCarriers",
                   "Set the number of Component carrier to use "
                   "If it is more than one and m_useCa is false, it will raise an error ",
                   UintegerValue (1),
                   MakeUintegerAccessor (&LteHelper::m_noOfCcs),
                   MakeUintegerChecker<uint16_t> (1, 2))
  ;
  return tid;
}

void
LteHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  // for (uint16_t i = 0; i < m_downlinkChannel.size (); i++)
  //   {
  //     m_downlinkChannel.at (i) = 0;
  //   }
  m_downlinkChannel.clear ();
  m_uplinkChannel.clear ();   // = 0;
  Object::DoDispose ();
}

void
LteHelper::ChannelModelInitialized (void)
{
  //m_noOfCcs = 2;
  ////NS_LOG_UNCOND("LteHelper::ChannelModelInitialized::m_noOfCcs=" << m_noOfCcs);
  // Channel Object (i.e. Ptr<SpecturmChannel>) are within a vector
  // PathLossModel Objects are vectors --> in InstallSingleEnb we will set the frequency
  NS_LOG_FUNCTION (this << m_noOfCcs);

  for (uint16_t i = 0; i < m_noOfCcs; i++)
    {
      Ptr<SpectrumChannel> m_downlinkChannelElem = m_channelFactory.Create<SpectrumChannel> ();
      Ptr<SpectrumChannel> m_uplinkChannelElem = m_channelFactory.Create<SpectrumChannel> ();

      Ptr<Object> m_downlinkPathlossModelElem = m_dlPathlossModelFactory.Create ();
      Ptr<SpectrumPropagationLossModel> dlSplm = m_downlinkPathlossModelElem->GetObject<SpectrumPropagationLossModel> ();
      if (dlSplm != 0)
        {
          NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in DL");
          m_downlinkChannelElem->AddSpectrumPropagationLossModel (dlSplm);
        }
      else
        {
          NS_LOG_LOGIC (this << " using a PropagationLossModel in DL");
          Ptr<PropagationLossModel> dlPlm = m_downlinkPathlossModelElem->GetObject<PropagationLossModel> ();
          NS_ASSERT_MSG (dlPlm != 0, " " << m_downlinkPathlossModelElem << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
          m_downlinkChannelElem->AddPropagationLossModel (dlPlm);
        }

      Ptr<Object> m_uplinkPathlossModelElem = m_ulPathlossModelFactory.Create ();
      Ptr<SpectrumPropagationLossModel> ulSplm = m_uplinkPathlossModelElem->GetObject<SpectrumPropagationLossModel> ();
      if (ulSplm != 0)
        {
          NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in UL");
          m_uplinkChannelElem->AddSpectrumPropagationLossModel (ulSplm);
        }
      else
        {
          NS_LOG_LOGIC (this << " using a PropagationLossModel in UL");
          Ptr<PropagationLossModel> ulPlm = m_uplinkPathlossModelElem->GetObject<PropagationLossModel> ();
          NS_ASSERT_MSG (ulPlm != 0, " " << m_uplinkPathlossModelElem << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
          m_uplinkChannelElem->AddPropagationLossModel (ulPlm);
        }
      if (!m_fadingModelType.empty ())
        {
          m_fadingModule = m_fadingModelFactory.Create<SpectrumPropagationLossModel> ();
          m_fadingModule->Initialize ();
          m_downlinkChannelElem->AddSpectrumPropagationLossModel (m_fadingModule);
          m_uplinkChannelElem->AddSpectrumPropagationLossModel (m_fadingModule);
        }
      m_downlinkChannel.push_back (m_downlinkChannelElem);
      m_uplinkChannel.push_back (m_uplinkChannelElem);
      m_uplinkPathlossModel.push_back (m_uplinkPathlossModelElem);
      m_downlinkPathlossModel.push_back (m_downlinkPathlossModelElem);
    }
}

void
LteHelper::SetEpcHelper (Ptr<EpcHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_epcHelper = h;
}

void
LteHelper::SetSchedulerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_schedulerFactory = ObjectFactory ();
  m_schedulerFactory.SetTypeId (type);
}

std::string
LteHelper::GetSchedulerType () const
{
  return m_schedulerFactory.GetTypeId ().GetName ();
}

void
LteHelper::SetSchedulerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_schedulerFactory.Set (n, v);
}

std::string
LteHelper::GetFfrAlgorithmType () const
{
  return m_ffrAlgorithmFactory.GetTypeId ().GetName ();
}

void
LteHelper::SetFfrAlgorithmType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_ffrAlgorithmFactory = ObjectFactory ();
  m_ffrAlgorithmFactory.SetTypeId (type);
}

void
LteHelper::SetFfrAlgorithmAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_ffrAlgorithmFactory.Set (n, v);
}

std::string
LteHelper::GetHandoverAlgorithmType () const
{
  return m_handoverAlgorithmFactory.GetTypeId ().GetName ();
}

void
LteHelper::SetHandoverAlgorithmType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_handoverAlgorithmFactory = ObjectFactory ();
  m_handoverAlgorithmFactory.SetTypeId (type);
}

void
LteHelper::SetHandoverAlgorithmAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_handoverAlgorithmFactory.Set (n, v);
}


std::string
LteHelper::GetEnbComponentCarrierManagerType () const
{
  return m_enbComponentCarrierManagerFactory.GetTypeId ().GetName ();
}

void
LteHelper::SetEnbComponentCarrierManagerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_enbComponentCarrierManagerFactory = ObjectFactory ();
  m_enbComponentCarrierManagerFactory.SetTypeId (type);
}

void
LteHelper::SetEnbComponentCarrierManagerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_enbComponentCarrierManagerFactory.Set (n, v);
}

std::string
LteHelper::GetUeComponentCarrierManagerType () const
{
  return m_ueComponentCarrierManagerFactory.GetTypeId ().GetName ();
}

void
LteHelper::SetUeComponentCarrierManagerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_ueComponentCarrierManagerFactory = ObjectFactory ();
  m_ueComponentCarrierManagerFactory.SetTypeId (type);
}

void
LteHelper::SetUeComponentCarrierManagerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_ueComponentCarrierManagerFactory.Set (n, v);
}

void
LteHelper::SetPathlossModelType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_dlPathlossModelFactory = ObjectFactory ();
  m_dlPathlossModelFactory.SetTypeId (type);
  m_ulPathlossModelFactory = ObjectFactory ();
  m_ulPathlossModelFactory.SetTypeId (type);
}

void
LteHelper::SetPathlossModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_dlPathlossModelFactory.Set (n, v);
  m_ulPathlossModelFactory.Set (n, v);
}

void
LteHelper::SetEnbDeviceAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_enbNetDeviceFactory.Set (n, v);
}


void
LteHelper::SetEnbAntennaModelType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_enbAntennaModelFactory.SetTypeId (type);
}

void
LteHelper::SetEnbAntennaModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_enbAntennaModelFactory.Set (n, v);
}

void
LteHelper::SetUeDeviceAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueNetDeviceFactory.Set (n, v);
}

void
LteHelper::SetUeAntennaModelType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaModelFactory.SetTypeId (type);
}

void
LteHelper::SetUeAntennaModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaModelFactory.Set (n, v);
}

void
LteHelper::SetFadingModel (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_fadingModelType = type;
  if (!type.empty ())
    {
      m_fadingModelFactory = ObjectFactory ();
      m_fadingModelFactory.SetTypeId (type);
    }
}

void
LteHelper::SetFadingModelAttribute (std::string n, const AttributeValue &v)
{
  m_fadingModelFactory.Set (n, v);
}

void
LteHelper::SetSpectrumChannelType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_channelFactory.SetTypeId (type);
}

void
LteHelper::SetSpectrumChannelAttribute (std::string n, const AttributeValue &v)
{
  m_channelFactory.Set (n, v);
}

void
LteHelper::SetEnbCc ( std::map< uint8_t, Ptr<ComponentCarrierEnb> > ccmap)
{
  NS_LOG_FUNCTION (this);
  m_enbComponentCarrierMap = ccmap;
}

NetDeviceContainer
LteHelper::InstallEnbDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  Initialize ();    // will run DoInitialize () if necessary
  ChannelModelInitialized ();
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleEnbDevice (node);  //crash12
      devices.Add (device);
    }

  return devices;
}

NetDeviceContainer
LteHelper::InstallUeDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleUeDevice (node);
      //NS_LOG_UNCOND("InstallSingleUeDevice left");
      devices.Add (device);
    }
  return devices;
}

Ptr<NetDevice>
LteHelper::InstallSingleEnbDevice (Ptr<Node> n)
{
  //m_noOfCcs = 1;
  ////NS_LOG_UNCOND("LteHelper::InstallSingleEnbDevice::m_noOfCcs=" << m_noOfCcs);
  NS_ABORT_MSG_IF (m_cellIdCounter == 65535, "max num eNBs exceeded");
  uint16_t cellId = ++m_cellIdCounter;
  std::map<uint8_t,Ptr<ComponentCarrierEnb> >::iterator it;
  // I create the LteEnbNetDevice object here
  // so if I need default information over CC specification I can take from it
  Ptr<LteEnbNetDevice> dev = m_enbNetDeviceFactory.Create<LteEnbNetDevice> ();
  Ptr<LteHandoverAlgorithm> handoverAlgorithm = m_handoverAlgorithmFactory.Create<LteHandoverAlgorithm> ();

  ////NS_LOG_UNCOND ("UlEarfcn=" << dev->GetUlEarfcn() << ", DlEarfcn=" << dev->GetDlEarfcn() << ", DlBandwidth=" << (uint16_t)dev->GetDlBandwidth() << ", UlBandwidth=" << (uint16_t)dev->GetUlBandwidth());

  if (m_useCa == false)
    {
      // we need to create a single Map now
      DoCreateEnbComponentCarrierMap (dev->GetUlEarfcn (), dev->GetDlEarfcn (), dev->GetDlBandwidth (), dev->GetUlBandwidth ());
    }
  else
    {
      ////NS_LOG_UNCOND ("LteHelper::InstallSingleEnbDevice(1)::m_enbComponentCarrierMap.size=" << m_enbComponentCarrierMap.size ());
      /*/LteHelper::DoCreateEnbComponentCarrierMap (uint16_t ulearfc, uint16_t dlearfcn, uint8_t ulbw, uint8_t dlbw)
      Ptr<CcHelper> cch = CreateObject<CcHelper> ();
      cch->m_numberOfComponentCarries = 1;
      cch->m_ulEarfcn = dev->GetUlEarfcn ();
      cch->m_dlEarfcn = dev->GetDlEarfcn ();
      cch->m_dlBandwidth = dev->GetDlBandwidth ();
      cch->m_ulBandwidth = dev->GetUlBandwidth ();
      m_enbComponentCarrierMap = cch->EquallySpacedCcs ();*/
      DoCreateEnbComponentCarrierMap (dev->GetUlEarfcn (), dev->GetDlEarfcn (), dev->GetDlBandwidth (), dev->GetUlBandwidth ());
      ////NS_LOG_UNCOND ("LteHelper::InstallSingleEnbDevice(1)::m_enbComponentCarrierMap.size=" << m_enbComponentCarrierMap.size ());
      NS_ABORT_MSG_IF (m_enbComponentCarrierMap.empty () == true, "You have to either define a valid Carrier Component MAP or disable Ca");
    }
  
  uint16_t counter = 0;
  for (it = m_enbComponentCarrierMap.begin (); it != m_enbComponentCarrierMap.end (); ++it)
    {

      Ptr<LteSpectrumPhy> dlPhy = CreateObject<LteSpectrumPhy> ();
      Ptr<LteSpectrumPhy> ulPhy = CreateObject<LteSpectrumPhy> ();
      Ptr<LteEnbPhy> phy = CreateObject<LteEnbPhy> (dlPhy, ulPhy);
      
      it->second->m_phy = phy;
      Ptr<LteHarqPhy> harq = Create<LteHarqPhy> ();
      dlPhy->SetHarqPhyModule (harq);
      ulPhy->SetHarqPhyModule (harq);
      phy->SetHarqPhyModule (harq);

      Ptr<LteChunkProcessor> pCtrl = Create<LteChunkProcessor> ();
      pCtrl->AddCallback (MakeCallback (&LteEnbPhy::GenerateCtrlCqiReport, phy));
      ulPhy->AddCtrlSinrChunkProcessor (pCtrl);   // for evaluating SRS UL-CQI

      Ptr<LteChunkProcessor> pData = Create<LteChunkProcessor> ();
      pData->AddCallback (MakeCallback (&LteEnbPhy::GenerateDataCqiReport, phy));
      pData->AddCallback (MakeCallback (&LteSpectrumPhy::UpdateSinrPerceived, ulPhy));
      ulPhy->AddDataSinrChunkProcessor (pData);   // for evaluating PUSCH UL-CQI

      Ptr<LteChunkProcessor> pInterf = Create<LteChunkProcessor> ();
      pInterf->AddCallback (MakeCallback (&LteEnbPhy::ReportInterference, phy));
      ulPhy->AddInterferenceDataChunkProcessor (pInterf);   // for interference power tracing

      dlPhy->SetChannel (m_downlinkChannel.at (counter));
      ulPhy->SetChannel (m_uplinkChannel.at (counter));
      counter++;

      Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
      NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling LteHelper::InstallUeDevice ()");
      dlPhy->SetMobility (mm);
      ulPhy->SetMobility (mm);

      Ptr<AntennaModel> antenna = (m_enbAntennaModelFactory.Create ())->GetObject<AntennaModel> ();
      NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
      dlPhy->SetAntenna (antenna);
      ulPhy->SetAntenna (antenna);
     
    }

  Ptr<LteEnbComponentCarrierManager> ccm = m_enbComponentCarrierManagerFactory.Create<LteEnbComponentCarrierManager> ();
  ccm->SetComponentCarrierNumber (m_noOfCcs);
  Ptr<LteEnbRrc> rrc = CreateObject<LteEnbRrc> ();
  rrc->m_numberOfComponentCarriers = m_noOfCcs;
  rrc->InitializeSap ();

  if (m_useIdealRrc)
    {
      Ptr<LteEnbRrcProtocolIdeal> rrcProtocol = CreateObject<LteEnbRrcProtocolIdeal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetCellId (cellId);
    }
  else
    {
      Ptr<LteEnbRrcProtocolReal> rrcProtocol = CreateObject<LteEnbRrcProtocolReal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetCellId (cellId);
    }

  if (m_epcHelper != 0)
    {
      EnumValue epsBearerToRlcMapping;
      rrc->GetAttribute ("EpsBearerToRlcMapping", epsBearerToRlcMapping);
      // it does not make sense to use RLC/SM when also using the EPC
      if (epsBearerToRlcMapping.Get () == LteEnbRrc::RLC_SM_ALWAYS)
        {
          rrc->SetAttribute ("EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
        }
    }

  rrc->SetLteHandoverManagementSapProvider (handoverAlgorithm->GetLteHandoverManagementSapProvider ());
  handoverAlgorithm->SetLteHandoverManagementSapUser (rrc->GetLteHandoverManagementSapUser ());

  // it = m_enbComponentCarrierMap.begin ();
 
  // rrc->SetLteMacSapProvider (it->second->GetMac ()->GetLteMacSapProvider ());
  // this RRC attribute it is used to connect each new RLC instance with the MAC layer
  // in this new architecture the ComponentCarrierManager acts as a proxy, so
  // each RLC istance see always the same LteMacSapProvider
  rrc->SetLteMacSapProvider (ccm->GetLteMacSapProvider ());
  bool ccmTest;
  uint16_t tmpCounter = 0;

  ////NS_LOG_UNCOND("LteHelper::InstallSingleEnbDevice(2)::m_enbComponentCarrierMap.size=" << m_enbComponentCarrierMap.size ());

  for (it = m_enbComponentCarrierMap.begin (); it != m_enbComponentCarrierMap.end (); ++it)
    {
      ////NS_LOG_UNCOND("LteHelper::InstallSingleEnbDevice::GetPhy ()" << it->second->GetPhy() << ", GetMac()=" << it->second->GetMac() );
      it->second->GetPhy ()->SetLteEnbCphySapUser (rrc->GetLteEnbCphySapUser (tmpCounter));
      rrc->SetLteEnbCphySapProvider (it->second->GetPhy ()->GetLteEnbCphySapProvider (), tmpCounter);

      rrc->SetLteEnbCmacSapProvider (it->second->GetMac ()->GetLteEnbCmacSapProvider (), tmpCounter );//crash1 in GetLteEnbCmacSapProvider()
      it->second->GetMac ()->SetLteEnbCmacSapUser (rrc->GetLteEnbCmacSapUser (tmpCounter));
      //FFR SAP
      it->second->GetFfMacScheduler ()->SetLteFfrSapProvider (it->second->GetFfrAlgorithm ()->GetLteFfrSapProvider ());
      it->second->GetFfrAlgorithm ()->SetLteFfrSapUser (it->second->GetFfMacScheduler ()->GetLteFfrSapUser ());
      rrc->SetLteFfrRrcSapProvider (it->second->GetFfrAlgorithm ()->GetLteFfrRrcSapProvider (), tmpCounter);
      it->second->GetFfrAlgorithm ()->SetLteFfrRrcSapUser (rrc->GetLteFfrRrcSapUser (tmpCounter));  //crash11
      //it->second->GetFfrAlgorithm ()->SetLteFfrRrcSapUser (rrc->GetLteFfrRrcSapUser (0));  //crash11
      //FFR SAP END

      // PHY <--> MAC SAP
      it->second->GetPhy ()->SetLteEnbPhySapUser (it->second->GetMac ()->GetLteEnbPhySapUser ());
      it->second->GetMac ()->SetLteEnbPhySapProvider (it->second->GetPhy ()->GetLteEnbPhySapProvider ());
      // PHY <--> MAC SAP END

      //Scheduler SAP
      it->second->GetMac ()->SetFfMacSchedSapProvider (it->second->GetFfMacScheduler ()->GetFfMacSchedSapProvider ());
      it->second->GetMac ()->SetFfMacCschedSapProvider (it->second->GetFfMacScheduler ()->GetFfMacCschedSapProvider ());

      it->second->GetFfMacScheduler ()->SetFfMacSchedSapUser (it->second->GetMac ()->GetFfMacSchedSapUser ());
      it->second->GetFfMacScheduler ()->SetFfMacCschedSapUser (it->second->GetMac ()->GetFfMacCschedSapUser ());
      // Scheduler SAP END

      it->second->GetMac ()->SetLteUlCcmMacSapUser (ccm->GetLteUlCcmMacSapUser ());
      ccm->SetUlCcmMacSapProviders (it->first, it->second->GetMac ()->GetLteUlCcmMacSapProvider ());

      // insert the LteMacSapUser pointer in the ComponentCarrierManager
      // it is needed to properly mapped the Logical Channels to the ComponentCarrier
      
      ccmTest = ccm->SetComponentCarrierMacSapProviders (it->first, it->second->GetMac ()->GetLteMacSapProvider());

      if (ccmTest == false)
        {
          NS_FATAL_ERROR ("Error in SetComponentCarrierMacSapProviders");
        }
      tmpCounter++;
    }

  //ComponentCarrierManager SAP InstallSingleEnbDevice
  rrc->SetLteCcmRrcSapProvider (ccm->GetLteCcmRrcSapProvider ()); // eNB
  ccm->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  ccm->SetComponentCarrierNumber (m_enbComponentCarrierMap.size ());
  //ComponentCarrierManager SAP END

  dev->SetNode (n);
  dev->SetAttribute ("CellId", UintegerValue (cellId));
  //dev->SetAttribute ("LteEnbPhy", PointerValue (it->second->GetPhy ()));// this will be a pointer to a map of phy
  //dev->SetAttribute("ComponentCarrierMap", ObjectMapValue (m_enbComponentCarrierMap));
  //std::cout << "m_enbComponentCarrierMap.size () " << (uint16_t)m_enbComponentCarrierMap.size () << std::endl;
  dev->SetCcMap (m_enbComponentCarrierMap);
  //dev->SetAttribute ("LteEnbMac", PointerValue (mac));
  it = m_enbComponentCarrierMap.begin ();
  // This attribute is not used (just for some tests)
  dev->SetAttribute ("FfMacScheduler", PointerValue (it->second->GetFfMacScheduler ()));
  dev->SetAttribute ("LteEnbRrc", PointerValue (rrc));
  dev->SetAttribute ("LteHandoverAlgorithm", PointerValue (handoverAlgorithm));
  dev->SetAttribute ("LteFfrAlgorithm", PointerValue (it->second->GetFfrAlgorithm ()));
/*  piotr
    .AddAttribute ("LteFfrAlgorithm",
                   "The FFR algorithm associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_ffrAlgorithm),
                   MakePointerChecker <LteFfrAlgorithm> ())
    .AddAttribute ("LteEnbComponentCarrierManager",
                   "The RRC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_componentCarrierManager),
                   MakePointerChecker <LteEnbComponentCarrierManager> ())
    .AddAttribute ("ComponentCarrierMap", "List of all Component Carrier.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&LteEnbNetDevice::m_ccMap),
                   MakeObjectMapChecker<ComponentCarrierEnb> ())

*/
  dev->SetAttribute ("LteEnbComponentCarrierManager", PointerValue (ccm));

  if (m_isAnrEnabled)
    {
      Ptr<LteAnr> anr = CreateObject<LteAnr> (cellId);
      rrc->SetLteAnrSapProvider (anr->GetLteAnrSapProvider ());
      anr->SetLteAnrSapUser (rrc->GetLteAnrSapUser ());
      dev->SetAttribute ("LteAnr", PointerValue (anr));
    }
  counter = 0;
  for (it = m_enbComponentCarrierMap.begin (); it != m_enbComponentCarrierMap.end (); ++it)
    {
      Ptr<LteEnbPhy> tmpPhy;
      tmpPhy = it->second->GetPhy ();
      tmpPhy->SetDevice (dev);
      tmpPhy->GetUlSpectrumPhy ()->SetDevice (dev);
      tmpPhy->GetDlSpectrumPhy ()->SetDevice (dev);
      tmpPhy->GetUlSpectrumPhy ()->SetLtePhyRxDataEndOkCallback (MakeCallback (&LteEnbPhy::PhyPduReceived, tmpPhy));
      tmpPhy->GetUlSpectrumPhy ()->SetLtePhyRxCtrlEndOkCallback (MakeCallback (&LteEnbPhy::ReceiveLteControlMessageList, tmpPhy));
      tmpPhy->GetUlSpectrumPhy ()->SetLtePhyUlHarqFeedbackCallback (MakeCallback (&LteEnbPhy::ReceiveLteUlHarqFeedback, tmpPhy));
      NS_LOG_LOGIC ("set the propagation model frequencies");
      double dlFreq = LteSpectrumValueHelper::GetCarrierFrequency (it->second->m_dlEarfcn);
      NS_LOG_LOGIC ("DL freq: " << dlFreq);
      bool dlFreqOk = m_downlinkPathlossModel.at (counter)->SetAttributeFailSafe ("Frequency", DoubleValue (dlFreq));
      if (!dlFreqOk)
        {
          NS_LOG_WARN ("DL propagation model does not have a Frequency attribute");
        }

      double ulFreq = LteSpectrumValueHelper::GetCarrierFrequency (it->second->m_ulEarfcn);

      NS_LOG_LOGIC ("UL freq: " << ulFreq);
      bool ulFreqOk = m_uplinkPathlossModel.at (counter)->SetAttributeFailSafe ("Frequency", DoubleValue (ulFreq));
      if (!ulFreqOk)
        {
          NS_LOG_WARN ("UL propagation model does not have a Frequency attribute");
        }

      counter++;
    }  //end for
  dev->Initialize (); //ncrash3
  n->AddDevice (dev);
  counter = 0;
  for (it = m_enbComponentCarrierMap.begin (); it != m_enbComponentCarrierMap.end (); ++it)
    {
      m_uplinkChannel.at (counter)->AddRx (it->second->GetPhy ()->GetUlSpectrumPhy ());
    }
  rrc->SetForwardUpCallback (MakeCallback (&LteEnbNetDevice::Receive, dev));


  if (m_epcHelper != 0)
    {
      NS_LOG_INFO ("adding this eNB to the EPC");
      m_epcHelper->AddEnb (n, dev, dev->GetCellId ());
      Ptr<EpcEnbApplication> enbApp = n->GetApplication (0)->GetObject<EpcEnbApplication> ();
      NS_ASSERT_MSG (enbApp != 0, "cannot retrieve EpcEnbApplication");

      // S1 SAPs
      rrc->SetS1SapProvider (enbApp->GetS1SapProvider ());
      enbApp->SetS1SapUser (rrc->GetS1SapUser ());

      // X2 SAPs
      Ptr<EpcX2> x2 = n->GetObject<EpcX2> ();
      x2->SetEpcX2SapUser (rrc->GetEpcX2SapUser ());
      rrc->SetEpcX2SapProvider (x2->GetEpcX2SapProvider ());
    }

  return dev;
}

Ptr<NetDevice>
LteHelper::InstallSingleUeDevice (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this);

  Ptr<LteUeNetDevice> dev = m_ueNetDeviceFactory.Create<LteUeNetDevice> ();
  std::map<uint8_t, Ptr<ComponentCarrierUe> > m_ueComponentCarrierMap;
  std::map<uint8_t, Ptr<ComponentCarrierEnb> >::iterator itCcMap;
  NS_ABORT_MSG_IF (m_enbComponentCarrierMap.empty () == true, "If CA is not enabled, before call this method you have to install Enbs --> InstallEnbDevice()");
  for (itCcMap = m_enbComponentCarrierMap.begin (); itCcMap != m_enbComponentCarrierMap.end (); ++itCcMap)
    {
      Ptr <ComponentCarrierUe> cc =  CreateObject<ComponentCarrierUe> ();
      cc->SetUlBandwidth ( itCcMap->second->GetUlBandwidth ());
      cc->SetDlBandwidth ( itCcMap->second->GetDlBandwidth ());
      cc->m_dlEarfcn = itCcMap->second->m_dlEarfcn;
      cc->m_ulEarfcn = itCcMap->second->m_ulEarfcn;
      cc->SetAsPrimary (itCcMap->second->IsPrimary ());
      Ptr<LteUeMac> mac = CreateObject<LteUeMac> ();
      cc->SetMac (mac);
      // cc->GetPhy ()->Initialize (); // it is initialized within the LteUeNetDevice::DoInitialize ()
      m_ueComponentCarrierMap.insert (std::pair<uint8_t, Ptr<ComponentCarrierUe> > (itCcMap->first, cc));
    }
  std::map<uint8_t, Ptr<ComponentCarrierUe> >::iterator it;
  uint16_t counter = 0;
  for (it = m_ueComponentCarrierMap.begin (); it != m_ueComponentCarrierMap.end (); ++it)
    {
      Ptr<LteSpectrumPhy> dlPhy = CreateObject<LteSpectrumPhy> ();
      Ptr<LteSpectrumPhy> ulPhy = CreateObject<LteSpectrumPhy> ();

      Ptr<LteUePhy> phy = CreateObject<LteUePhy> (dlPhy, ulPhy);
      it->second->m_phy = phy;
      Ptr<LteHarqPhy> harq = Create<LteHarqPhy> ();
      dlPhy->SetHarqPhyModule (harq);
      ulPhy->SetHarqPhyModule (harq);
      phy->SetHarqPhyModule (harq);

      Ptr<LteChunkProcessor> pRs = Create<LteChunkProcessor> ();
      pRs->AddCallback (MakeCallback (&LteUePhy::ReportRsReceivedPower, phy));
      dlPhy->AddRsPowerChunkProcessor (pRs);

      Ptr<LteChunkProcessor> pInterf = Create<LteChunkProcessor> ();
      pInterf->AddCallback (MakeCallback (&LteUePhy::ReportInterference, phy));
      dlPhy->AddInterferenceCtrlChunkProcessor (pInterf);   // for RSRQ evaluation of UE Measurements

      Ptr<LteChunkProcessor> pCtrl = Create<LteChunkProcessor> ();
      pCtrl->AddCallback (MakeCallback (&LteSpectrumPhy::UpdateSinrPerceived, dlPhy));
      dlPhy->AddCtrlSinrChunkProcessor (pCtrl);

      Ptr<LteChunkProcessor> pData = Create<LteChunkProcessor> ();
      pData->AddCallback (MakeCallback (&LteSpectrumPhy::UpdateSinrPerceived, dlPhy));
      dlPhy->AddDataSinrChunkProcessor (pData);

      if (m_usePdschForCqiGeneration)
        {
          // CQI calculation based on PDCCH for signal and PDSCH for interference
          pCtrl->AddCallback (MakeCallback (&LteUePhy::GenerateMixedCqiReport, phy));
          Ptr<LteChunkProcessor> pDataInterf = Create<LteChunkProcessor> ();
          pDataInterf->AddCallback (MakeCallback (&LteUePhy::ReportDataInterference, phy));
          dlPhy->AddInterferenceDataChunkProcessor (pDataInterf);
        }
      else
        {
          // CQI calculation based on PDCCH for both signal and interference
          pCtrl->AddCallback (MakeCallback (&LteUePhy::GenerateCtrlCqiReport, phy));
        }



      dlPhy->SetChannel (m_downlinkChannel.at (counter));
      ulPhy->SetChannel (m_uplinkChannel.at (counter));
      counter++;

      Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
      NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling LteHelper::InstallUeDevice ()");
      dlPhy->SetMobility (mm);
      ulPhy->SetMobility (mm);

      Ptr<AntennaModel> antenna = (m_ueAntennaModelFactory.Create ())->GetObject<AntennaModel> ();
      NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
      dlPhy->SetAntenna (antenna);
      ulPhy->SetAntenna (antenna);
    }
  Ptr<LteUeComponentCarrierManager> ccm = m_ueComponentCarrierManagerFactory.Create<LteUeComponentCarrierManager> ();
  // somehow the LteUeComponentCarrierManager object disappers after leaving this procedure !!!
  //Ptr<LteUeComponentCarrierManager> ccm = (m_ueComponentCarrierManagerFactory.Create ())->GetObject<LteUeComponentCarrierManager> ();
  ccm->GetLteCcmRrcSapProvider()->AmIalive();
  Ptr<LteUeRrc> rrc = CreateObject<LteUeRrc> ();
  //rrc->m_numberOfComponentCarriers = m_noOfCcs;
  //rrc->InitializeSap ();

  //ComponentCarrierManager SAP InstallSingleUeDevice
  rrc->SetLteCcmRrcSapProvider (ccm->GetLteCcmRrcSapProvider ()); // ue sets the m_ccmRrcSapProvider
  ccm->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  //ComponentCarrierManager SAP END
  rrc->GetLteCcmRrcSapProvider()->AmIalive();
  //NS_LOG_UNCOND("LteHelper::InstallSingleUeDevice::m_ccmRrcSapProvider=" << rrc->GetLteCcmRrcSapProvider() << ", " << ccm->GetLteCcmRrcSapProvider());

  if (m_useIdealRrc)
    {
      Ptr<LteUeRrcProtocolIdeal> rrcProtocol = CreateObject<LteUeRrcProtocolIdeal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }
  else
    {
      Ptr<LteUeRrcProtocolReal> rrcProtocol = CreateObject<LteUeRrcProtocolReal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }

  if (m_epcHelper != 0)
    {
      rrc->SetUseRlcSm (false);
    }
  Ptr<EpcUeNas> nas = CreateObject<EpcUeNas> ();
 
  nas->SetAsSapProvider (rrc->GetAsSapProvider ());
  rrc->SetAsSapUser (nas->GetAsSapUser ());

 
  uint16_t tmpCounter = 0;
  for (it=m_ueComponentCarrierMap.begin (); it != m_ueComponentCarrierMap.end (); ++it)
    {
      rrc->SetLteUeCmacSapProvider (it->second->GetMac ()->GetLteUeCmacSapProvider (), tmpCounter);
      it->second->GetMac ()->SetLteUeCmacSapUser (rrc->GetLteUeCmacSapUser (tmpCounter));
      tmpCounter++;
    }
  it = m_ueComponentCarrierMap.begin ();
  it->second->GetPhy ()->SetLteUePhySapUser ( it->second->GetMac ()->GetLteUePhySapUser ());
  it->second->GetMac ()->SetLteUePhySapProvider ( it->second->GetPhy ()->GetLteUePhySapProvider ());
  rrc->SetLteMacSapProvider (it->second->GetMac ()->GetLteMacSapProvider ());
  it->second->GetPhy ()->SetLteUeCphySapUser (rrc->GetLteUeCphySapUser ());
  rrc->SetLteUeCphySapProvider (it->second->GetPhy ()->GetLteUeCphySapProvider ());

  NS_ABORT_MSG_IF (m_imsiCounter >= 0xFFFFFFFF, "max num UEs exceeded");
  uint64_t imsi = ++m_imsiCounter;


  dev->SetNode (n);
  dev->SetAttribute ("Imsi", UintegerValue (imsi));
  //dev->SetAttribute ("LteUePhy", PointerValue (phy));
  //dev->SetAttribute ("ComponentCarrierMapUe", ObjectMapValue (m_ueComponentCarrierMap));
  dev->SetCcMap (m_ueComponentCarrierMap);
  // dev->SetAttribute ("LteUeMac", PointerValue (mac));
  dev->SetAttribute ("LteUeRrc", PointerValue (rrc));
  dev->SetAttribute ("LteUeComponentCarrierManager", PointerValue (ccm));
  dev->SetAttribute ("EpcUeNas", PointerValue (nas));
  counter = 0;
  for (it = m_ueComponentCarrierMap.begin (); it != m_ueComponentCarrierMap.end (); ++it)
    {
      Ptr<LteUePhy> tmpPhy;
      tmpPhy = it->second->GetPhy ();
      tmpPhy->SetDevice (dev);
      tmpPhy->GetUlSpectrumPhy ()->SetDevice (dev);
      tmpPhy->GetDlSpectrumPhy ()->SetDevice (dev);
      tmpPhy->GetDlSpectrumPhy ()->SetLtePhyRxDataEndOkCallback (MakeCallback (&LteUePhy::PhyPduReceived, tmpPhy));
      tmpPhy->GetDlSpectrumPhy ()->SetLtePhyRxCtrlEndOkCallback (MakeCallback (&LteUePhy::ReceiveLteControlMessageList, tmpPhy));
      tmpPhy->GetDlSpectrumPhy ()->SetLtePhyRxPssCallback (MakeCallback (&LteUePhy::ReceivePss, tmpPhy));
      tmpPhy->GetDlSpectrumPhy ()->SetLtePhyDlHarqFeedbackCallback (MakeCallback (&LteUePhy::ReceiveLteDlHarqFeedback, tmpPhy));
    }

  nas->SetDevice (dev);

  n->AddDevice (dev);

  nas->SetForwardUpCallback (MakeCallback (&LteUeNetDevice::Receive, dev));

  if (m_epcHelper != 0)
    {
      m_epcHelper->AddUe (dev, dev->GetImsi ());
    }

  dev->Initialize ();

  rrc->GetLteCcmRrcSapProvider()->AmIalive();
  //NS_LOG_UNCOND("LteHelper::InstallSingleUeDevice::m_ccmRrcSapProvider=" << rrc->GetLteCcmRrcSapProvider() << ", " << ccm->GetLteCcmRrcSapProvider());

  //NS_LOG_UNCOND("LteHelper::InstallSingleUeDevice::dev=" << dev);
  dev->test();            //2crash-good
  Ptr<LteUeRrc> ueRrc = dev->GetRrc ();
  ueRrc->test();          //2crash-good

  //NS_LOG_UNCOND("Leaving LteHelper::InstallSingleUeDevice");
  return dev;
}

void
LteHelper::test(NodeContainer c)
{
  //NS_LOG_UNCOND("LteHelper::test=" << this);
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = node->GetDevice (0);
      //NS_LOG_UNCOND("LteHelper::test=" << device->GetTypeId ());
      //Ptr<MobilityModel> mm = node->GetObject<MobilityModel> ();
      Ptr<LteUeNetDevice> dev = device->GetObject<LteUeNetDevice> ();
      //NS_LOG_UNCOND("BAD-LteHelper::InstallSingleUeDevice::dev=" << dev);
      dev->test();          //2crash-bad
      Ptr<LteUeRrc> ueRrc = dev->GetRrc ();             // good
      Ptr<LteUeRrc> rrc = dev->GetObject<LteUeRrc> ();  // bad
      //NS_LOG_UNCOND("BAD-LteHelper::test::rrc=" << ueRrc << ", " << rrc);
      ueRrc->test();        //2crash-bad
    }
}

void
LteHelper::Attach (NetDeviceContainer ueDevices)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      Attach (*i);
    }
}

void
LteHelper::Attach (Ptr<NetDevice> ueDevice)
{
  NS_LOG_FUNCTION (this);

  if (m_epcHelper == 0)
    {
      NS_FATAL_ERROR ("This function is not valid without properly configured EPC");
    }

  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice> ();
  if (ueLteDevice == 0)
    {
      NS_FATAL_ERROR ("The passed NetDevice must be an LteUeNetDevice");
    }

  // initiate cell selection
  Ptr<EpcUeNas> ueNas = ueLteDevice->GetNas ();
  NS_ASSERT (ueNas != 0);
  uint16_t dlEarfcn = ueLteDevice->GetDlEarfcn ();
  ueNas->StartCellSelection (dlEarfcn);

  // instruct UE to immediately enter CONNECTED mode after camping
  ueNas->Connect ();

  // activate default EPS bearer
  m_epcHelper->ActivateEpsBearer (ueDevice, ueLteDevice->GetImsi (),
                                  EpcTft::Default (),
                                  EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
}

void
LteHelper::Attach (NetDeviceContainer ueDevices, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      Attach (*i, enbDevice);
    }
}

void
LteHelper::Attach (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this);
  //enbRrc->SetCellId (enbDevice->GetObject<LteEnbNetDevice> ()->GetCellId ());

  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice> ();
  Ptr<LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<LteEnbNetDevice> ();

  Ptr<EpcUeNas> ueNas = ueLteDevice->GetNas ();
  ueNas->Connect (enbLteDevice->GetCellId (), enbLteDevice->GetDlEarfcn ());

  if (m_epcHelper != 0)
    {
      // activate default EPS bearer
      m_epcHelper->ActivateEpsBearer (ueDevice, ueLteDevice->GetImsi (), EpcTft::Default (), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
    }

  // tricks needed for the simplified LTE-only simulations
  if (m_epcHelper == 0)
    {
      ueDevice->GetObject<LteUeNetDevice> ()->SetTargetEnb (enbDevice->GetObject<LteEnbNetDevice> ());
    }
}

void
LteHelper::AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      AttachToClosestEnb (*i, enbDevices);
    }
}

void
LteHelper::AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (enbDevices.GetN () > 0, "empty enb device container");
  Vector uepos = ueDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  double minDistance = std::numeric_limits<double>::infinity ();
  Ptr<NetDevice> closestEnbDevice;
  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      Vector enbpos = (*i)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
      double distance = CalculateDistance (uepos, enbpos);
      if (distance < minDistance)
        {
          minDistance = distance;
          closestEnbDevice = *i;
        }
    }
  NS_ASSERT (closestEnbDevice != 0);
  Attach (ueDevice, closestEnbDevice);
}

uint8_t
LteHelper::ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      uint8_t bearerId = ActivateDedicatedEpsBearer (*i, bearer, tft);
      return bearerId;
    }
  return 0;
}


uint8_t
LteHelper::ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "dedicated EPS bearers cannot be set up when the EPC is not used");

  uint64_t imsi = ueDevice->GetObject<LteUeNetDevice> ()->GetImsi ();
  uint8_t bearerId = m_epcHelper->ActivateEpsBearer (ueDevice, imsi, tft, bearer);
  return bearerId;
}

/**
 * \ingroup lte
 *
 * DrbActivatior allows user to activate bearers for UEs
 * when EPC is not used. Activation function is hooked to
 * the Enb RRC Connection Estabilished trace source. When
 * UE change its RRC state to CONNECTED_NORMALLY, activation
 * function is called and bearer is activated.
 */
class DrbActivator : public SimpleRefCount<DrbActivator>
{
public:
  /**
   * DrbActivator Constructor
   *
   * \param ueDevice the UeNetDevice for which bearer will be activated
   * \param bearer the bearer configuration
   */
  DrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer);

  /**
   * Function hooked to the Enb RRC Connection Established trace source
   * Fired upon successful RRC connection establishment.
   *
   * \param a DrbActivator object
   * \param context
   * \param imsi
   * \param cellId
   * \param rnti
   */
  static void ActivateCallback (Ptr<DrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti);

  /**
   * Procedure firstly checks if bearer was not activated, if IMSI
   * from trace source equals configured one and if UE is really
   * in RRC connected state. If all requirements are met, it performs
   * bearer activation.
   *
   * \param imsi
   * \param cellId
   * \param rnti
   */
  void ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti);
private:
  /**
   * Bearer can be activated only once. This value stores state of
   * bearer. Initially is set to false and changed to true during
   * bearer activation.
   */
  bool m_active;
  /**
   * UeNetDevice for which bearer will be activated
   */
  Ptr<NetDevice> m_ueDevice;
  /**
   * Configuration of bearer which will be activated
   */
  EpsBearer m_bearer;
  /**
   * imsi the unique UE identifier
   */
  uint64_t m_imsi;
};

DrbActivator::DrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer)
  : m_active (false),
    m_ueDevice (ueDevice),
    m_bearer (bearer),
    m_imsi (m_ueDevice->GetObject<LteUeNetDevice> ()->GetImsi ())
{
}

void
DrbActivator::ActivateCallback (Ptr<DrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (a << context << imsi << cellId << rnti);
  a->ActivateDrb (imsi, cellId, rnti);
}

void
DrbActivator::ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti << m_active);
  if ((!m_active) && (imsi == m_imsi))
    {
      Ptr<LteUeRrc> ueRrc = m_ueDevice->GetObject<LteUeNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetState () == LteUeRrc::CONNECTED_NORMALLY);
      uint16_t rnti = ueRrc->GetRnti ();
      Ptr<LteEnbNetDevice> enbLteDevice = m_ueDevice->GetObject<LteUeNetDevice> ()->GetTargetEnb ();
      Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetObject<LteEnbNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetCellId () == enbLteDevice->GetCellId ());
      Ptr<UeManager> ueManager = enbRrc->GetUeManager (rnti);
      NS_ASSERT (ueManager->GetState () == UeManager::CONNECTED_NORMALLY
                 || ueManager->GetState () == UeManager::CONNECTION_RECONFIGURATION);
      EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = m_bearer;
      params.bearerId = 0;
      params.gtpTeid = 0;   // don't care
      enbRrc->GetS1SapUser ()->DataRadioBearerSetupRequest (params);
      m_active = true;
    }
}


void
LteHelper::ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice);
  NS_ASSERT_MSG (m_epcHelper == 0, "this method must not be used when the EPC is being used");

  // Normally it is the EPC that takes care of activating DRBs
  // when the UE gets connected. When the EPC is not used, we achieve
  // the same behavior by hooking a dedicated DRB activation function
  // to the Enb RRC Connection Established trace source


  Ptr<LteEnbNetDevice> enbLteDevice = ueDevice->GetObject<LteUeNetDevice> ()->GetTargetEnb ();

  std::ostringstream path;
  path << "/NodeList/" << enbLteDevice->GetNode ()->GetId ()
       << "/DeviceList/" << enbLteDevice->GetIfIndex ()
       << "/LteEnbRrc/ConnectionEstablished";
  Ptr<DrbActivator> arg = Create<DrbActivator> (ueDevice, bearer);
  Config::Connect (path.str (), MakeBoundCallback (&DrbActivator::ActivateCallback, arg));

}

void
LteHelper::AddX2Interface (NodeContainer enbNodes)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "X2 interfaces cannot be set up when the EPC is not used");

  for (NodeContainer::Iterator i = enbNodes.Begin (); i != enbNodes.End (); ++i)
    {
      for (NodeContainer::Iterator j = i + 1; j != enbNodes.End (); ++j)
        {
          AddX2Interface (*i, *j);
        }
    }
}

void
LteHelper::AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("setting up the X2 interface");

  m_epcHelper->AddX2Interface (enbNode1, enbNode2);
}

void
LteHelper::HandoverRequest (Time hoTime, Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, Ptr<NetDevice> targetEnbDev)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetEnbDev);
  NS_ASSERT_MSG (m_epcHelper, "Handover requires the use of the EPC - did you forget to call LteHelper::SetEpcHelper () ?");
  Simulator::Schedule (hoTime, &LteHelper::DoHandoverRequest, this, ueDev, sourceEnbDev, targetEnbDev);
}

void
LteHelper::DoHandoverRequest (Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, Ptr<NetDevice> targetEnbDev)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetEnbDev);

  uint16_t targetCellId = targetEnbDev->GetObject<LteEnbNetDevice> ()->GetCellId ();
  Ptr<LteEnbRrc> sourceRrc = sourceEnbDev->GetObject<LteEnbNetDevice> ()->GetRrc ();
  uint16_t rnti = ueDev->GetObject<LteUeNetDevice> ()->GetRrc ()->GetRnti ();
  sourceRrc->SendHandoverRequest (rnti, targetCellId);
}

void
LteHelper::DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice,Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);
  NS_ASSERT_MSG (m_epcHelper != 0, "Dedicated EPS bearers cannot be de-activated when the EPC is not used");
  NS_ASSERT_MSG (bearerId != 1, "Default bearer cannot be de-activated until and unless and UE is released");

  DoDeActivateDedicatedEpsBearer (ueDevice, enbDevice, bearerId);
}

void
LteHelper::DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);

  //Extract IMSI and rnti
  uint64_t imsi = ueDevice->GetObject<LteUeNetDevice> ()->GetImsi ();
  uint16_t rnti = ueDevice->GetObject<LteUeNetDevice> ()->GetRrc ()->GetRnti ();


  Ptr<LteEnbRrc> enbRrc = enbDevice->GetObject<LteEnbNetDevice> ()->GetRrc ();

  enbRrc->DoSendReleaseDataRadioBearer (imsi,rnti,bearerId);
}

void
LteHelper::DoCreateEnbComponentCarrierMap (uint16_t ulearfc, uint16_t dlearfcn, uint8_t ulbw, uint8_t dlbw)
{
  std::map<uint8_t,Ptr<ComponentCarrierEnb> >::iterator it;
  Ptr<CcHelper> cch = CreateObject<CcHelper> ();
  cch->m_numberOfComponentCarries = 1;
  cch->m_ulEarfcn = ulearfc;
  cch->m_dlEarfcn = dlearfcn;
  cch->m_dlBandwidth = dlbw;
  cch->m_ulBandwidth = ulbw;
  m_enbComponentCarrierMap = cch->EquallySpacedCcs ();
 
  //it = m_enbComponentCarrierMap.begin ();
  for (it = m_enbComponentCarrierMap.begin (); it != m_enbComponentCarrierMap.end (); ++it)
    {
      Ptr<LteEnbMac> mac = CreateObject<LteEnbMac> ();
      Ptr<FfMacScheduler> sched = m_schedulerFactory.Create<FfMacScheduler> ();
      Ptr<LteFfrAlgorithm> ffrAlgorithm = m_ffrAlgorithmFactory.Create<LteFfrAlgorithm> ();

      it->second->SetMac (mac);
      it->second->SetFfMacScheduler (sched);
      //it->second->SetFfrAlgorithm (ffrAlgorithm);
      it->second->m_ffrAlgorithm = ffrAlgorithm;
    }
}


void
LteHelper::ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      ActivateDataRadioBearer (*i, bearer);
    }
}

void
LteHelper::EnableLogComponents (void)
{
  LogComponentEnable ("LteHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LteEnbRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("LteUeRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("LteEnbMac", LOG_LEVEL_ALL);
  LogComponentEnable ("LteUeMac", LOG_LEVEL_ALL);
  LogComponentEnable ("LteRlc", LOG_LEVEL_ALL);
  LogComponentEnable ("LteRlcUm", LOG_LEVEL_ALL);
  LogComponentEnable ("LteRlcAm", LOG_LEVEL_ALL);
  LogComponentEnable ("RrFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("PfFfMacScheduler", LOG_LEVEL_ALL);

  LogComponentEnable ("LtePhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LteEnbPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LteUePhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LteSpectrumValueHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LteSpectrumPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LteInterference", LOG_LEVEL_ALL);
  LogComponentEnable ("LteChunkProcessor", LOG_LEVEL_ALL);

  std::string propModelStr = m_dlPathlossModelFactory.GetTypeId ().GetName ().erase (0,5).c_str ();
  LogComponentEnable ("LteNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("LteUeNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("LteEnbNetDevice", LOG_LEVEL_ALL);

  LogComponentEnable ("RadioBearerStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("LteStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("MacStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("PhyTxStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("PhyRxStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("PhyStatsCalculator", LOG_LEVEL_ALL);


}

void
LteHelper::EnableTraces (void)
{
  EnablePhyTraces ();
  EnableMacTraces ();
  EnableRlcTraces ();
  EnablePdcpTraces ();
}

void
LteHelper::EnableRlcTraces (void)
{
  NS_ASSERT_MSG (m_rlcStats == 0, "please make sure that LteHelper::EnableRlcTraces is called at most once");
  m_rlcStats = CreateObject<RadioBearerStatsCalculator> ("RLC");
  m_radioBearerStatsConnector.EnableRlcStats (m_rlcStats);
}

int64_t
LteHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  if ((m_fadingModule != 0) && (m_fadingStreamsAssigned == false))
    {
      Ptr<TraceFadingLossModel> tflm = m_fadingModule->GetObject<TraceFadingLossModel> ();
      if (tflm != 0)
        {
          currentStream += tflm->AssignStreams (currentStream);
          m_fadingStreamsAssigned = true;
        }
    }
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<LteEnbNetDevice> lteEnb = DynamicCast<LteEnbNetDevice> (netDevice);
      if (lteEnb)
        {
          std::map< uint8_t, Ptr <ComponentCarrierEnb> > tmpMap = lteEnb->GetCcMap ();
          std::map< uint8_t, Ptr <ComponentCarrierEnb> >::iterator it;
          it = tmpMap.begin ();
          Ptr<LteSpectrumPhy> dlPhy = it->second->GetPhy ()->GetDownlinkSpectrumPhy ();
          Ptr<LteSpectrumPhy> ulPhy = it->second->GetPhy ()->GetUplinkSpectrumPhy ();
          currentStream += dlPhy->AssignStreams (currentStream);
          currentStream += ulPhy->AssignStreams (currentStream);
        }
      Ptr<LteUeNetDevice> lteUe = DynamicCast<LteUeNetDevice> (netDevice);
      if (lteUe)
        {
          std::map< uint8_t, Ptr <ComponentCarrierUe> > tmpMap = lteUe->GetCcMap ();
          std::map< uint8_t, Ptr <ComponentCarrierUe> >::iterator it;
          it = tmpMap.begin ();
          Ptr<LteSpectrumPhy> dlPhy = it->second->GetPhy ()->GetDownlinkSpectrumPhy ();
          Ptr<LteSpectrumPhy> ulPhy = it->second->GetPhy ()->GetUplinkSpectrumPhy ();
          Ptr<LteUeMac> ueMac = lteUe->GetMac ();
          currentStream += dlPhy->AssignStreams (currentStream);
          currentStream += ulPhy->AssignStreams (currentStream);
          currentStream += ueMac->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}


void
LteHelper::EnablePhyTraces (void)
{
  EnableDlPhyTraces ();
  EnableUlPhyTraces ();
  EnableDlTxPhyTraces ();
  EnableUlTxPhyTraces ();
  EnableDlRxPhyTraces ();
  EnableUlRxPhyTraces ();
}

void
LteHelper::EnableDlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/LteEnbPhy/DlPhyTransmission",
                   MakeBoundCallback (&PhyTxStatsCalculator::DlPhyTransmissionCallback, m_phyTxStats));
}

void
LteHelper::EnableUlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/LteUePhy/UlPhyTransmission",
                   MakeBoundCallback (&PhyTxStatsCalculator::UlPhyTransmissionCallback, m_phyTxStats));
}

void
LteHelper::EnableDlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/LteUePhy/DlSpectrumPhy/DlPhyReception",
                   MakeBoundCallback (&PhyRxStatsCalculator::DlPhyReceptionCallback, m_phyRxStats));
}

void
LteHelper::EnableUlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/LteEnbPhy/UlSpectrumPhy/UlPhyReception",
                   MakeBoundCallback (&PhyRxStatsCalculator::UlPhyReceptionCallback, m_phyRxStats));
}


void
LteHelper::EnableMacTraces (void)
{
  EnableDlMacTraces ();
  EnableUlMacTraces ();
}


void
LteHelper::EnableDlMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/LteEnbMac/DlScheduling",
                   MakeBoundCallback (&MacStatsCalculator::DlSchedulingCallback, m_macStats));
}

void
LteHelper::EnableUlMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/LteEnbMac/UlScheduling",
                   MakeBoundCallback (&MacStatsCalculator::UlSchedulingCallback, m_macStats));
}

void
LteHelper::EnableDlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr",
                   MakeBoundCallback (&PhyStatsCalculator::ReportCurrentCellRsrpSinrCallback, m_phyStats));
}

void
LteHelper::EnableUlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr",
                   MakeBoundCallback (&PhyStatsCalculator::ReportUeSinr, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/LteEnbPhy/ReportInterference",
                   MakeBoundCallback (&PhyStatsCalculator::ReportInterference, m_phyStats));

}

Ptr<RadioBearerStatsCalculator>
LteHelper::GetRlcStats (void)
{
  return m_rlcStats;
}

void
LteHelper::EnablePdcpTraces (void)
{
  NS_ASSERT_MSG (m_pdcpStats == 0, "please make sure that LteHelper::EnablePdcpTraces is called at most once");
  m_pdcpStats = CreateObject<RadioBearerStatsCalculator> ("PDCP");
  m_radioBearerStatsConnector.EnablePdcpStats (m_pdcpStats);
}

Ptr<RadioBearerStatsCalculator>
LteHelper::GetPdcpStats (void)
{
  return m_pdcpStats;
}

} // namespace ns3
