/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef __PHY_NR_TRANSPORT_DCI__H
#define __PHY_NR_TRANSPORT_DCI__H

#include "PHY/defs_gNB.h"
#include "PHY/NR_REFSIG/nr_refsig.h"
#include "nfapi/open-nFAPI/nfapi/public_inc/nfapi_nr_interface.h"

void nr_generate_dci_top(processingData_L1tx_t *msgTx, int slot, int txdataF_offset);

int16_t find_nr_pdcch(int frame,int slot, PHY_VARS_gNB *gNB,find_type_t type);

void nr_fill_dci(PHY_VARS_gNB *gNB,
                 int frame,
                 int slot,
		 nfapi_nr_dl_tti_pdcch_pdu *pdcch_pdu);

int16_t find_nr_ul_dci(int frame,int slot, PHY_VARS_gNB *gNB,find_type_t type);

void nr_fill_ul_dci(PHY_VARS_gNB *gNB,
		    int frame,
		    int slot,
		    nfapi_nr_ul_dci_request_pdus_t *pdcch_pdu);

void nr_fill_reg_list(int cce_list[MAX_DCI_CORESET][NR_MAX_PDCCH_AGG_LEVEL * NR_NB_REG_PER_CCE], nfapi_nr_dl_tti_pdcch_pdu_rel15_t *pdcch_pdu_rel15);

#endif //__PHY_NR_TRANSPORT_DCI__H
