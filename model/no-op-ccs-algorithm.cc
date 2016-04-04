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

#include "no-op-ccs-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NoOpCcsAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (NoOpCcsAlgorithm);


NoOpCcsAlgorithm::NoOpCcsAlgorithm ()
  : m_componentCarrierManagementSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_componentCarrierManagementSapProvider = new MemberLteComponentCarrierManagementSapProvider<NoOpCcsAlgorithm> (this);
}


NoOpCcsAlgorithm::~NoOpCcsAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}


void
NoOpCcsAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_componentCarrierManagementSapProvider;
}


TypeId
NoOpCcsAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NoOpCcsAlgorithm")
    .SetParent<LteCcsAlgorithm> ()
    .SetGroupName("Lte")
    .AddConstructor<NoOpCcsAlgorithm> ()
  ;
  return tid;
}


void
NoOpCcsAlgorithm::SetLteComponentCarrierManagementSapUser (LteComponentCarrierManagementSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_componentCarrierManagementSapUser = s;
}


LteComponentCarrierManagementSapProvider*
NoOpCcsAlgorithm::GetLteComponentCarrierManagementSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_componentCarrierManagementSapProvider;
}


void
NoOpCcsAlgorithm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  LteCcsAlgorithm::DoInitialize ();
}


void
NoOpCcsAlgorithm::DoReportUeMeas (uint16_t rnti,
                                       LteRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
}


} // end of namespace ns3
