/*
 * e2p.h
 *
 *  Created on: 5 apr. 2017
 *      Author: VM
 */

#ifndef E2P_H_
#define E2P_H_
#include <stdbool.h>

// E2P device address.
#define E2P_RIIC_ADDRESS   (0xA0) //50

// UUID 48-bit.
#define E2P_NODE_ADDRESS   (0xFA)

#if 0
// E2P memory organization. 24AA08E
#define E2P_PAGE_COUNT       (16)
#define E2P_PAGE_SIZE        (16)
#define E2P_PAGE_OFFSET      (4)
#else
// E2P memory organization. 24AA02E48T
//#define E2P_PAGE_COUNT       (16)
//#define E2P_PAGE_SIZE         (8)
//#define E2P_PAGE_OFFSET       (3)
#endif


// E2P memory organization.
#define E2P_PAGE_COUNT       (16)
#define E2P_PAGE_SIZE        (8)
#define E2P_PAGE_OFFSET      (3)




// Public function prototypes.
bool e2p_get_serial_no(uint8_t *p_dst);
uint8_t WriteStruct2eerom(i2c_ee_t to_save);
uint8_t ReadStruct2eerom(i2c_ee_t * to_read);

bool e2p_read(uint8_t address, uint8_t *p_dst, uint8_t len);
bool e2p_write(uint8_t address, uint8_t *p_src, uint8_t len);



#endif /* E2P_H_ */
