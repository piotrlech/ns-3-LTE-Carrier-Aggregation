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

#ifndef LTE_CCS_ALGORITHM_H
#define LTE_CCS_ALGORITHM_H

#include <ns3/object.h>
#include <ns3/lte-rrc-sap.h>

namespace ns3 {


class LteComponentCarrierManagementSapUser;
class LteComponentCarrierManagementSapProvider;


/**
 * \brief The abstract base class of a Carrier Component Selection (CCS) algorithm that operates using
 *        the ComponentCarrier Management SAP interface.
 *
 * CCS algorithm receives measurement reports from an eNode RRC instance
 * and tells the eNodeB RRC instance when to do a CCS.
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
 *     Ptr<LteCcsAlgorithm> p = ...;
 *     u->SetLteComponentCarrierManagementSapProvider (p->GetLteComponentCarrierManagementSapProvider ());
 *     p->SetLteComponentCarrierManagementSapUser (u->GetLteComponentCarrierManagementSapUser ());
 *
 * However, user rarely needs to use the above code, since it has already been
 * taken care by LteHelper::InstallEnbDevice.
 *
 * \sa LteComponentCarrierManagementSapProvider, LteComponentCarrierManagementSapUser
 */
class LteCcsAlgorithm : public Object
{
public:
  LteCcsAlgorithm ();
  virtual ~LteCcsAlgorithm ();

  // inherited from Object
  static TypeId GetTypeId ();

  /**
   * \brief Set the "user" part of the ComponentCarrier Management SAP interface that
   *        this ComponentCarrier algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an LteEnbRrc instance
   */
  virtual void SetLteComponentCarrierManagementSapUser (LteComponentCarrierManagementSapUser* s) = 0;

  /**
   * \brief Export the "provider" part of the ComponentCarrier Management SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an LteEnbRrc instance
   */
  virtual LteComponentCarrierManagementSapProvider* GetLteComponentCarrierManagementSapProvider () = 0;

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

}; // end of class LteCcsAlgorithm


} // end of namespace ns3


#endif /* LTE_CCS_ALGORITHM_H */
