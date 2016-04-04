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

#ifndef NO_OP_CCS_ALGORITHM_H
#define NO_OP_CCS_ALGORITHM_H

#include <ns3/lte-ccs-algorithm.h>
#include <ns3/lte-componentcarrier-management-sap.h>
#include <ns3/lte-rrc-sap.h>

namespace ns3 {


/**
 * \brief Component carrier selection algorithm implementation which simply does nothing.
 *
 * Selecting this Component carrier selectionalgorithm is equivalent to disabling automatic
 * triggering of Component carrier selection. This is the default choice.
 *
 * To enable automatic Component carrier selection, please select another Component carrier selection algorithm, i.e.,
 * another child class of LteCcsAlgorithm.
 */
class NoOpCcsAlgorithm : public LteCcsAlgorithm
{
public:
  /// Creates a No-op CCS algorithm instance.
  NoOpCcsAlgorithm ();

  virtual ~NoOpCcsAlgorithm ();

  // inherited from Object
  static TypeId GetTypeId ();

  // inherited from LteCcsAlgorithm
  virtual void SetLteComponentCarrierManagementSapUser (LteComponentCarrierManagementSapUser* s);
  virtual LteComponentCarrierManagementSapProvider* GetLteComponentCarrierManagementSapProvider ();

  // let the forwarder class access the protected and private members
  friend class MemberLteComponentCarrierManagementSapProvider<NoOpCcsAlgorithm>;

protected:
  // inherited from Object
  virtual void DoInitialize ();
  virtual void DoDispose ();

  // inherited from LteCcsAlgorithm as a Component Carrier Management SAP implementation
  void DoReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults);

private:
  /// Interface to the eNodeB RRC instance.
  LteComponentCarrierManagementSapUser* m_componentCarrierManagementSapUser;
  /// Receive API calls from the eNodeB RRC instance.
  LteComponentCarrierManagementSapProvider* m_componentCarrierManagementSapProvider;

}; // end of class NoOpCcsAlgorithm


} // end of namespace ns3


#endif /* NO_OP_CCS_ALGORITHM_H */
