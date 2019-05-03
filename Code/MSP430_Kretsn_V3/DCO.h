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
    BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
    DCOCTL = CALDCO_1MHZ;
}

#endif /* DCO_H_ */
