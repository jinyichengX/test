#ifndef __IPGUI_GRAPHYIC2_H__
#define __IPGUI_GRAPHYIC2_H__

#define IPGUI_PIXEL_BITS 6
#define IPGUI_PIXEL_PRECI (1 << IPGUI_PIXEL_BITS) /* 像素精度1/(1 << 6) = 1/64 ≈ 0.015625 */
#define IPGUI_PIXEL_AREA  (IPGUI_PIXEL_PRECI * IPGUI_PIXEL_PRECI) 
/* stroke render parameters begin */

/*   
      line join bevel                 line join miter                 line join round  
                                            .                             . .
                                           . .                           . . .
         . . . . .                        . . .                        . . . . .
        . . . . . .                      . . . .                      . . . . . .
       . . . . . . .                    . . . . .                    . . .   . . .
      . . . . . . . .                  . . . . . .                  . . .     . . .
     . . . .   . . . .                . . .   . . .                . . .       . . .
    . . . .     . . . .              . . .     . . .              . . .         . . .
   . . . .       . . . .            . . .       . . .            . . .           . . .
  . . . .         . . . .          . . .         . . .          . . .             . . .
*/

#endif