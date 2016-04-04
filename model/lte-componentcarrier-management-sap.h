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

#ifndef LTE_COMPONENTCARRIER_MANAGEMENT_SAP_H
#define LTE_COMPONENTCARRIER_MANAGEMENT_SAP_H

#include <ns3/lte-rrc-sap.h>

namespace ns3 {


/**
 * \brief Service Access Point (SAP) offered by the component carrier selection algorithm instance
 *        to the eNodeB RRC instance.
 *
 * This is the *Component Carrier Management SAP Provider*, i.e., the part of the SAP
 * that contains the component carrier selection algorithm methods called by the eNodeB RRC
 * instance.
 */
class LteComponentCarrierManagementSapProvider
{
public:
  virtual ~LteComponentCarrierManagementSapProvider ();

  /**
   * \brief Send a UE measurement report to component carrier selection algorithm.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param measResults a single report of one measurement identity
   *
   * The received measurement report is a result of the UE measurement
   * configuration previously configured by calling
   * LteComponentCarrierManagementSapProvider::AddUeMeasReportConfigForComponentCarrierSelection. The report
   * may be stored and utilised for the purpose of making ComponentCarrier decision.
   */
  virtual void ReportUeMeas (uint16_t rnti,
                             LteRrcSap::MeasResults measResults) = 0;

}; // end of class LteComponentCarrierManagementSapProvider


/**
 * \brief Service Access Point (SAP) offered by the eNodeB RRC instance to the
 *        ComponentCarrier algorithm instance.
 *
 * This is the *Component Carrier Management SAP User*, i.e., the part of the SAP that
 * contains the eNodeB RRC methods called by the component carrier selection algorithm instance.
 */
class LteComponentCarrierManagementSapUser
{
public:
  virtual ~LteComponentCarrierManagementSapUser ();

  /**
   * \brief Request a certain reporting configuration to be fulfilled by the UEs
   *        attached to the eNodeB entity.
   * \param reportConfig the UE measurement reporting configuration
   * \return the measurement identity associated with this newly added
   *         reporting configuration
   *
   * The eNodeB RRC entity is expected to configure the same reporting
   * configuration in each of the attached UEs. When later in the simulation a
   * UE measurement report is received from a UE as a result of this
   * configuration, the eNodeB RRC entity shall forward this report to the
   * ComponentCarrier algorithm through the LteComponentCarrierManagementSapUser::ReportUeMeas
   * SAP function.
   *
   * \note This function is only valid before the simulation begins.
   */
  virtual uint8_t AddUeMeasReportConfigForComponentCarrier (LteRrcSap::ReportConfigEutra reportConfig) = 0;

  /**
   * \brief Instruct the eNodeB RRC entity to prepare a ComponentCarrier.
   * \param rnti Radio Network Temporary Identity, an integer identifying the
   *             UE which shall perform the ComponentCarrier
   * \param targetCellId the cell ID of the target eNodeB
   *
   * This function is used by the ComponentCarrier algorithm entity when a ComponentCarrier
   * decision has been reached.
   *
   * The process to produce the decision is up to the implementation of ComponentCarrier
   * algorithm. It is typically based on the reported UE measurements, which are
   * received through the LteComponentCarrierManagementSapProvider::ReportUeMeas function.
   */
  virtual void TriggerComponentCarrier (uint16_t rnti, uint16_t targetCellId) = 0;

}; // end of class LteComponentCarrierManagementSapUser



/**
 * \brief Template for the implementation of the LteComponentCarrierManagementSapProvider
 *        as a member of an owner class of type C to which all methods are
 *        forwarded.
 */
template <class C>
class MemberLteComponentCarrierManagementSapProvider : public LteComponentCarrierManagementSapProvider
{
public:
  MemberLteComponentCarrierManagementSapProvider (C* owner);

  // inherited from LteComponentCarrierManagemenrSapProvider
  virtual void ReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults);

private:
  MemberLteComponentCarrierManagementSapProvider ();
  C* m_owner;

}; // end of class MemberLteComponentCarrierManagementSapProvider


template <class C>
MemberLteComponentCarrierManagementSapProvider<C>::MemberLteComponentCarrierManagementSapProvider (C* owner)
  : m_owner (owner)
{
}


template <class C>
void
MemberLteComponentCarrierManagementSapProvider<C>::ReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults)
{
  m_owner->DoReportUeMeas (rnti, measResults);
}



/**
 * \brief Template for the implementation of the LteComponentCarrierManagementSapUser
 *        as a member of an owner class of type C to which all methods are
 *        forwarded.
 */
template <class C>
class MemberLteComponentCarrierManagementSapUser : public LteComponentCarrierManagementSapUser
{
public:
  MemberLteComponentCarrierManagementSapUser (C* owner);

  // inherited from LteComponentCarrierManagementSapUser
  virtual uint8_t AddUeMeasReportConfigForComponentCarrier (LteRrcSap::ReportConfigEutra reportConfig);
  virtual void TriggerComponentCarrier (uint16_t rnti, uint16_t targetCellId);

private:
  MemberLteComponentCarrierManagementSapUser ();
  C* m_owner;

}; // end of class MemberLteAnrSapUser


template <class C>
MemberLteComponentCarrierManagementSapUser<C>::MemberLteComponentCarrierManagementSapUser (C* owner)
  : m_owner (owner)
{
}


template <class C>
uint8_t
MemberLteComponentCarrierManagementSapUser<C>::AddUeMeasReportConfigForComponentCarrier (LteRrcSap::ReportConfigEutra reportConfig)
{
  return m_owner->DoAddUeMeasReportConfigForComponentCarrier (reportConfig);
}


template <class C>
void
MemberLteComponentCarrierManagementSapUser<C>::TriggerComponentCarrier (uint16_t rnti, uint16_t targetCellId)
{
  return m_owner->DoTriggerComponentCarrier (rnti, targetCellId);
}


} // end of namespace ns3


#endif /* LTE_COMPONENTCARRIER_MANAGEMENT_SAP_H */
