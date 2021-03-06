/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 *
 * 1-1	0.01	UDP	1.0.0.2<->7.0.0.2
 * 0-2	0.02	UDP	1.0.0.2<->7.0.0.2
 * 0-3	0.12	GTP	10.0.0.6<->10.0.0.5	UDP	1.0.0.2<->7.0.0.2
 * 2-2	0.12	GTP	10.0.0.6<->10.0.0.5	UDP	1.0.0.2<->7.0.0.2
 *
 * remoteHost(1-1:1.0.0.2) ---- (1.0.0.1:0-2)pgw(0-3: 10.0.0.6) ---- (10.0.0.5:2-2)eNb >>>> (7.0.0.2:3)ue
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */

NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");

static void
RxDrop (Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}

int
main (int argc, char *argv[])
{

  uint16_t numberOfNodes = 1;
  double simTime = 5.1;
  double distance = 60.0;
  double udpPacketInterval = 100;
  uint32_t tcpPktSize = 1400;         //in bytes. 1458 to prevent fragments
  uint16_t verbose = 2;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", udpPacketInterval);
  cmd.AddValue ("pktSize", "Packet size in bytes", tcpPktSize);
  cmd.AddValue ("verbose", "Verbose level", verbose);
  cmd.Parse(argc, argv);

  //Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  simTime = 1;
  if (true)
    {
      Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (true));
      Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (2));
    }

  /*Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1000000));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (100));
  //pio-NS_LOG_FUNCTION (this << GetName ());
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  //pio-Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (m_useIdealRrc));

  //Disable Uplink Power Control
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));*/

  /**
   * Initialize Simulation Scenario: 1 eNB and m_nUser UEs
   */


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  /*Config::SetDefault ("ns3::RrFfMacScheduler::HarqEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::PfFfMacScheduler::HarqEnabled", BooleanValue (false));*/

  //lteHelper->SetSchedulerAttribute ("HarqEnabled", BooleanValue (false));

  //pio-lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
  //pio-lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (0.0));
  //pio-lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (0.0));
  //pio-lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (0.0));

  //pio-end

  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  //NS_LOG_UNCOND ("simTime=" << simTime << ", m_enbComponentCarriers=" << &CcHelper::m_numberOfComponentCarries);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);//crash3
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  //piotr
  //NS_LOG_UNCOND("BEFORE=" << ueNodes.Get(0)->GetDevice(0)->GetObject<LteUeNetDevice>()->GetRrc()->GetLteCcmRrcSapProvider());
  //ueNodes.Get(0)->GetDevice(0)->GetObject<LteUeNetDevice>()->GetRrc()->GetLteCcmRrcSapProvider()->AmIalive();
  //NS_LOG_UNCOND("AFTER=");
  Ptr<Node> ue = ueNodes.Get (0);
  //NS_LOG_UNCOND("ue=" << ue << "i=" << 0 << ", " << ue->GetDevice (0)->GetTypeId ());
  //Ptr<NetDevice> dev = ue->GetDevice (0);
  //lteHelper->test(ueNodes);         // 2crash

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < numberOfNodes; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
        // side effect: the default EPS bearer will be activated
        //enbLteDevs.Get(i)->SetAttribute ("DlBandwidth", UintegerValue (5));
      }

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ++ulPort;
      ++otherPort;
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));

      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(udpPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(udpPacketInterval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
      client.SetAttribute ("Interval", TimeValue (MilliSeconds(udpPacketInterval)));
      client.SetAttribute ("MaxPackets", UintegerValue(1000000));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      if (u+1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get(0)));
        }

      uint16_t tcpPort = 84;
      Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), tcpPort));
      PacketSinkHelper tcpSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
      // Configure application
      AddressValue remoteAddress (InetSocketAddress (ueIpIface.GetAddress (u), tcpPort));
      Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpPktSize));
      BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
      ftp.SetAttribute ("Remote", remoteAddress);
      ftp.SetAttribute ("SendSize", UintegerValue (tcpPktSize));
      ftp.SetAttribute ("MaxBytes", UintegerValue (0));
      //ApplicationContainer sourceApp = ftp.Install (source.Get (0));
      serverApps.Add (ftp.Install (remoteHost));
      //ftp client
      tcpSinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
      //ApplicationContainer sinkApp = sinkHelper.Install (sink);
      clientApps.Add (tcpSinkHelper.Install (ueNodes.Get(u)));
    }
  serverApps.Start (Seconds (0.1));
  clientApps.Start (Seconds (0.1));
  lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  if (verbose > 1)
    p2ph.EnablePcapAll("piotr-epc-first");

  if (verbose > 5)
    {
      ueLteDevs.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));
      enbLteDevs.Get(0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));
      //enbNodes.Get(0)->GetDevice(0)->GetObject<LtePdcp>()->TraceConnectWithoutContext ("RxPDU", MakeCallback (&RxDrop));
      //NS_LOG_UNCOND(enbNodes.Get(0)->GetDevice(0)->GetObject<LtePdcp>()->GetStatus().txSn);
      //NS_LOG_UNCOND("pdcp=" << ueNodes.Get(0)->GetDevice(0)->GetObject<LteUeNetDevice>()->GetRrc()->Get);
      LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
      LogComponentEnable("LtePdcp", LOG_LEVEL_ALL);
      LogComponentEnable("LteRlcAm", LOG_LEVEL_ALL);
      LogComponentEnable("LteRlcUm", LOG_LEVEL_ALL);
    }

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();
  return 0;

}

