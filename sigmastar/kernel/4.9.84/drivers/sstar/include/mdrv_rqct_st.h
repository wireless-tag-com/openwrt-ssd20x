/*
* mdrv_rqct_st.h- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#ifndef _MDRV_RQCT_ST_H_
#define _MDRV_RQCT_ST_H_

#define RQCTIF_MAJ              1   //!< major version: Major number of driver-I/F version.
#define RQCTIF_MIN              1   //!< minor version: Minor number of driver-I/F version.
#define RQCTIF_EXT              1   //!< extended code: Extended number of version. It should increase when "mdrv_rqct_io.h/mdrv_rqct_st.h" changed.

//! RQCT Interface version number.
#define RQCTIF_VERSION_ID       ((RQCTIF_MAJ<<22)|(RQCTIF_MIN<<12)|(RQCTIF_EXT))
//! Acquire version number.
#define RQCTIF_GET_VER(v)       (((v)>>12))
//! Acquire major version number.
#define RQCTIF_GET_MJR(v)       (((v)>>22)&0x3FF)
//! Acquire minor version number.
#define RQCTIF_GET_MNR(v)       (((v)>>12)&0x3FF)
//! Acquire extended number.
#define RQCTIF_GET_EXT(v)       (((v)>> 0)&0xFFF)

//! rqct-method : support 3 types - CQP, CBR, VBR.
enum rqct_method
{
    RQCT_METHOD_CQP=0,  //!< constant QP.
    RQCT_METHOD_CBR,    //!< constant bitrate.
    RQCT_METHOD_VBR,    //!< variable bitrate.
};

//! mvhe_superfrm_mode indicates super frame mechanism
enum rqct_superfrm_mode
{
    RQCT_SUPERFRM_NONE=0, //!< super frame mode none.
    RQCT_SUPERFRM_DISCARD, //!< super frame mode discard.
    RQCT_SUPERFRM_REENCODE, //!< super frame mode reencode.
};

//! rqct_conf is used to apply/query rq-configs during streaming period.
typedef union rqct_conf
{
    //! rqct-config type.
    enum rqct_conf_e
    {
        RQCT_CONF_SEQ=0,    //!< set sequence rate-control.
        RQCT_CONF_DQP,      //!< set delta-qp between I/P.
        RQCT_CONF_QPR,      //!< set range-qp.
        RQCT_CONF_LOG,      //!< turn on/off rqct log message.
        RQCT_CONF_PEN,      //!< penalties for mfe.
        RQCT_CONF_SPF,      //!< super frame settings.
        RQCT_CONF_LTR,      //!< long term reference setting
        RQCT_CONF_END,      //!< endof rqct-conf-enum.
    } type;                 //!< indicating config. type.

    //! set rqct seq. config.
    struct _seq
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_SEQ.
        enum rqct_method    i_method;   //!< indicating rqct-method.
        int                 i_period;   //!< ip-period.
        int                 i_leadqp;   //!< leadqp.
        int                 i_btrate;   //!< btrate.
    } seq;          //!< rqct configs of seq. setting.

    //! set rqct lt config
    struct _ltr
    {
        enum rqct_conf_e    i_type;         //!< i_type MUST be RQCT_CONF_LTR.
        int                 i_period;       //!< ltr period
    } ltr;

    //! set rqct dqp. config.
    struct _dqp
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_DQP.
        int                 i_dqp;      //!< dif-qp between I/P.
    } dqp;          //!< rqct configs of dqp. setting.

    //! set rqct qpr. config.
    struct _qpr
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_QPR.
        int                 i_iupperq;   //!< I frame upperq.
        int                 i_ilowerq;   //!< I frame lowerq.
        int                 i_pupperq;   //!< P frame upperq.
        int                 i_plowerq;   //!< P frame lowerq.
    } qpr;          //!< rqct configs of dqp. setting.

    //! set rqct log. config.
    struct _log
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_LOG.
        int                 b_logm;     //!< switch of log-message.
    } log;          //!< rqct configs of dqp. setting.

    //! set rqct pen. config.
    struct _pen
    {
        enum rqct_conf_e    i_type;     //!< i_type MUST be RQCT_CFG_PEN.
        short               b_i16pln;  //!< enable intra16 planar.
        short               i_peni4x;  //!< penalty intra4x4.
        short               i_peni16;  //!< penalty intra16.
        short               i_penint;  //!< penalty inter.
        short               i_penYpl;  //!< penalty planar luma.
        short               i_penCpl;  //!< penalty planar cbcr.
    } pen;          //!< rqct configs of dqp. setting.

    struct _spf
    {
        enum rqct_conf_e    i_type;         //!< i_type MUST be MVHE_PARM_SPF.
        enum rqct_superfrm_mode    i_spfrm;
        int  i_IFrmBitsThr;
        int  i_PFrmBitsThr;
        int  i_BFrmBitsThr;
    } spf;          //!< used to set super frame skip mode configuration.
} rqct_conf;

#endif//_MDRV_RQCT_ST_H_
//! @}
