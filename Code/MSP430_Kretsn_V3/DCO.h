/*
 * DCO.h
 *
 *  DCO header file
 *  Functions:
 *      -- Set DCO settings (change clock freq.)
 *
 *  Created on: 17 Jan 2018
 *      Author: Mandus
 */

#ifndef DCO_H_
#define DCO_H_

void DCO_Calib()
{
    if (CALBC1_1MHZ==0xFF)                  // If calibration constant erased
      {
        while(1);                               // do not load, trap CPU!!
      }
      DCOCTL = 0;                               // Select lowest DCOx and MODx settings
      BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1MHz
      DCOCTL = CALDCO_1MHZ;
      FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator

//    BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
//    DCOCTL = CALDCO_1MHZ;
}

#endif /* DCO_H_ */
