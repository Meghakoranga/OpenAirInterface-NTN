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

#ifndef __NR_MODULATION_H__
#define __NR_MODULATION_H__

#include <stdint.h>
#include "PHY/defs_nr_common.h"
#include "PHY/defs_gNB.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_ue.h"

#define DMRS_MOD_ORDER 2
/*Precoding matices: W[pmi][antenna_port][layer]*/
extern const char nr_W_1l_2p[6][2][1];
extern const char nr_W_2l_2p[3][2][2];
extern const char nr_W_1l_4p[28][4][1];
extern const char nr_W_2l_4p[22][4][2];
extern const char nr_W_3l_4p[7][4][3];
extern const char nr_W_4l_4p[5][4][4];
/*! \brief Perform NR modulation. TS 38.211 V15.4.0 subclause 5.1
  @param[in] in, Pointer to input bits
  @param[in] length, size of input bits
  @param[in] modulation_type, modulation order
  @param[out] out, complex valued modulated symbols
*/

void nr_modulation(const uint32_t *in,
                   uint32_t length,
                   uint16_t mod_order,
                   int16_t *out);

/*! \brief Perform NR layer mapping. TS 38.211 V15.4.0 subclause 7.3.1.3
  @param[in] mod_symbs, double Pointer to modulated symbols for each codeword
  @param[in] n_layers, number of layers
  @param[in] n_symbs, number of modulated symbols
  @param[out] tx_layers, modulated symbols for each layer
*/

void nr_layer_mapping(int nbCodes,
                      int encoded_len,
                      c16_t mod_symbs[nbCodes][encoded_len],
                      uint8_t n_layers,
                      int layerSz,
                      uint32_t n_symbs,
                      c16_t tx_layers[][layerSz]);

/*! \brief Perform NR layer mapping. TS 38.211 V15.4.0 subclause 7.3.1.3
  @param[in] ulsch_ue, double Pointer to NR_UE_ULSCH_t struct
  @param[in] n_layers, number of layers
  @param[in] n_symbs, number of modulated symbols
  @param[out] tx_layers, modulated symbols for each layer
*/
void nr_ue_layer_mapping(const c16_t *mod_symbs, const int n_layers, const int n_symbs, c16_t tx_layers[][n_symbs]);
/*!
\brief This function implements the OFDM front end processor on reception (FEP)
\param frame_parms Pointer to frame parameters
\param rxdata Pointer to input data in time domain
\param rxdataF Pointer to output data in frequency domain
\param symbol symbol within slot (0..12/14)
\param Ns Slot number (0..19)
\param sample_offset offset within rxdata (points to beginning of subframe)
*/

int nr_slot_fep_ul(NR_DL_FRAME_PARMS *frame_parms,
                   int32_t *rxdata,
                   int32_t *rxdataF,
                   unsigned char symbol,
                   unsigned char Ns,
                   int sample_offset);

/*!
\brief This function implements the dft transform precoding in PUSCH
\param z Pointer to output in frequnecy domain
\param d Pointer to input in time domain
\param Msc_PUSCH number of allocated data subcarriers
*/
void nr_dft(c16_t *z, c16_t *d, uint32_t Msc_PUSCH);

void nr_beam_precoding(c16_t **txdataF,
                       c16_t **txdataF_BF,
                       NR_DL_FRAME_PARMS *frame_parms,
                       int32_t ***beam_weights,
                       int slot,
                       int symbol,
                       int aa,
                       int nb_antenna_ports,
                       int offset);

void apply_nr_rotation_TX(const NR_DL_FRAME_PARMS *fp,
                          c16_t *txdataF,
                          const c16_t *symbol_rotation,
                          int slot,
                          int nb_rb,
                          int first_symbol,
                          int nsymb);

void perform_symbol_rotation(NR_DL_FRAME_PARMS *fp, double f0, c16_t *symbol_rotation);

void init_symbol_rotation(NR_DL_FRAME_PARMS *fp);

void init_timeshift_rotation(NR_DL_FRAME_PARMS *fp);

void apply_nr_rotation_RX(const NR_DL_FRAME_PARMS *frame_parms,
                          c16_t *rxdataF,
                          const c16_t *rot,
                          int slot,
                          int nb_rb,
                          int soffset,
                          int first_symbol,
                          int nsymb);

/*! \brief Perform NR precoding. TS 38.211 V15.4.0 subclause 6.3.1.5
  @param[in] datatx_F_precoding, Pointer to n_layers*re data array
  @param[in] prec_matrix, Pointer to precoding matrix
  @param[in] n_layers, number of DLSCH layers
*/
c16_t nr_layer_precoder(int sz, c16_t datatx_F_precoding[][sz], const char *prec_matrix, uint8_t n_layers, int32_t re_offset);

c16_t nr_layer_precoder_cm(int n_layers,
                           int symSz,
                           c16_t datatx_F_precoding[n_layers][symSz],
                           int ap,
                           nfapi_nr_pm_pdu_t *pmi_pdu,
                           int offset);

/*! \brief Precoding with SIMDe, txdataF_precoded[] = prec_matrix[] * txdataF_res_mapped[]
  @param[in]  txdataF_res_mapped Tx data after resource mapping, before precoding.
  @param[in]  prec_matrix        Weights of precoding matrix.
  @param[in]  re_cnt             Number of RE (sub carrier) to write to txdataF_precoded, should be multiple of 4.
  @param[out] txdataF_precoded   Precoded antenna data
*/
void nr_layer_precoder_simd(const int n_layers,
                            const int symSz,
                            const c16_t txdataF_res_mapped[n_layers][symSz],
                            const int ant,
                            const nfapi_nr_pm_pdu_t *pmi_pdu,
                            const int sc_offset,
                            const int re_cnt,
                            c16_t *txdataF_precoded);
#endif
