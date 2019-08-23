/*
 * frameUpdate.h
 *
 *  Frame update header file
 *  Functions:
 *      -- Handle progression of frame counter as well as start/stop of animations
 *
 *  Created on: 22 Aug 2019
 *      Author: Mandus
 */

#ifndef FRAMEUPDATE_H_
#define FRAMEUPDATE_H_

char img_halt = 0;
char img_loop = 0;
unsigned int  FRAME_CNTR = 0;   // Current frame that is displayed

char img_isRunning(){
    return (img_halt == 0) ? 0x01:0x00;
}

void img_start(struct imageDescriptor image){
    img_halt = 0;
    img_loop = image.loops;
    FRAME_CNTR = 0;
}

void img_stop(){img_halt = 1;}

void img_update(struct imageDescriptor image){
    // Increment frame counter
    if(image.mode == MODE_FRAME){
        FRAME_CNTR += 16;
    } else if (image.mode==MODE_SWEEP_UP || image.mode==MODE_SWEEP_DN) {
        FRAME_CNTR += 2;
    } else {
        FRAME_CNTR = 0;
    }

    // Reset counter if at image end
    if (FRAME_CNTR + 16 > image.size){
        if(img_loop > 9){
            // > 9 loops -> Loop infinitely
            FRAME_CNTR = 0;
        }else if(img_loop > 0){
            img_loop--;
            FRAME_CNTR = 0;
        }else{
            FRAME_CNTR = img.size - 16; // Move to end of image
            img_stop();
        }
    }
    // Calculate flash index of first element in current frame
    if(image.mode==MODE_SWEEP_DN){
        FRAME_OFFS = IMG_MEM_BASE + image.offset + image.size - (FRAME_CNTR+16);
    } else {
        FRAME_OFFS = IMG_MEM_BASE + image.offset + FRAME_CNTR;
    }
}

unsigned char img_find_chained(unsigned char chain){

    unsigned char n_ones = 0;
    // Find number of chained animations
    int i;
    for(i=0; i<sizeof(chain)*8; i++){
        if( (chain&(0x01<<i)) != 0x00)
            n_ones++;
    }
    if(n_ones == 0) return 0; // Should never happen, but will cause inf loop if not handled

    unsigned char rand_index = TA0R & 0x07; // Grab lowest byte (values 0-7 decimal) from timer 0, as pseudo-rng

    // Reduce rand_index until it is in range ]n_ones 0]
    while(rand_index >= n_ones)
        rand_index -= n_ones;

    rand_index++;

    for(i=0; i<sizeof(chain)*8; i++){
        if((chain&(0x01<<i)) != 0x00)
            rand_index--;
        if(rand_index == 0)
            return i;
    }

    return 0;
}

#endif /* FRAMEUPDATE_H_ */
