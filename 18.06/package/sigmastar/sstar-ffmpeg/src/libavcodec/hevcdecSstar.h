/**
 * @file
 * H.265/HEVC codec.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#ifndef AVCODEC_SSHEVCDEC_H
#define AVCODEC_SSHEVCDEC_H

#include <stdbool.h>

#include "libavutil/buffer.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/thread.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>


#include "avcodec.h"
#include "bswapdsp.h"
#include "cabac.h"
#include "get_bits.h"
#include "hevcpred.h"
#include "h2645_parse.h"
#include "hevc.h"
#include "hevc_ps.h"
#include "hevc_sei.h"
#include "hevcdsp.h"
#include "internal.h"
#include "thread.h"
#include "videodsp.h"


//sstar sdk lib
#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_vdec_extra.h"

#include "mi_common.h"
#include "mi_common_datatype.h"

#include "mi_sys.h"
#include "mi_sys_datatype.h"

//#include "mi_divp.h"
//#include "mi_divp_datatype.h"

//#include "mi_hdmi.h"
//#include "mi_hdmi_datatype.h"

//#include "mi_disp.h"
//#include "mi_disp_datatype.h"


typedef struct SsHevcContext {
    AVFrame *frame;
    AVCodecContext *avctx;
    H2645Packet pkt;

    int width;
    int height;
    int format;

    uint8_t * pkt_buf;
    int pkt_size;
    int64_t pts, dts;

    uint8_t *data;
    int data_size;
    int max_data_size;
    int find_header;

    // type of the first VCL NAL of the current frame
    int seq_decode;
    int64_t max_ra; 
    uint8_t eos, last_eos;
    int is_nalff;           ///< this flag is != 0 if bitstream is encapsulated
                            ///< as a format defined in 14496-15
    int nal_length_size;    ///< Number of bytes used for nal length (1, 2 or 4)
    int64_t time_sec, time_wait;
} SsHevcContext;


#endif

