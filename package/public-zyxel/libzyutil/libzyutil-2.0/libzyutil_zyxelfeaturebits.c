#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#define _GNU_SOURCE

#include <linux/zy_feature_bits.h>
#include "zcfg_common.h"
#include "zcfg_debug.h"
#include "libzyutil_zyxelfeaturebits.h"
#include "zos_api.h"

/*!
 *  Get the value of Zyxel Feature Bits (/proc/zyxel/zy_feature_bits).
 *
 *  @param[out] value       Value of zy_feature_bits.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
static zcfgRet_t _zyxelfeaturebits_allValueGet(unsigned int *value);
static zcfgRet_t _zyxelfeaturebits_allValueGet(unsigned int *value)
{
    FILE *fp = NULL;
    char str[9] = {0};

    *value = 0;
    fp = ZOS_FOPEN(PROC_ZY_FEATURE_BITS, "r");
    if ( !fp )
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : Cannot open %s.\n", __func__, PROC_ZY_FEATURE_BITS);
        return ZCFG_INTERNAL_ERROR;
    }

    memset(str, 0, sizeof(str));
    if (fgets(str, sizeof(str), fp) == NULL)
    {
        ZOS_FCLOSE(fp);
        zcfgLog(ZCFG_LOG_ERR, "%s : Get string fail.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    sscanf(str, "%08x", value);
    ZOS_FCLOSE(fp);

    return ZCFG_SUCCESS;
}
/*!
 *  Get the offset value of Zyxel Feature Bits.
 *
 *  @param[in] offset       Offset of specify bit.
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
static zcfgRet_t _zyxelfeaturebits_offsetValueGet(unsigned int offset, int *value);
static zcfgRet_t _zyxelfeaturebits_offsetValueGet(unsigned int offset, int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    unsigned int featurebits;

    *value = 0;
    ret = _zyxelfeaturebits_allValueGet(&featurebits);
    if (ret == ZCFG_INTERNAL_ERROR)
    {
        return ret;
    }

    *value = ((featurebits & offset) ? 1 : 0);

    return ret;
}
/*!
 *  Get the mask value of Zyxel Feature Bits.
 *
 *  @param[in] mask         Mask of specify bits.
 *  @param[out] value       Specify bits of zy_feature_bits.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
static zcfgRet_t _zyxelfeaturebits_maskValueGet(unsigned int mask, unsigned int *value);
static zcfgRet_t _zyxelfeaturebits_maskValueGet(unsigned int mask, unsigned int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    unsigned int featurebits;

    *value = 0;
    ret = _zyxelfeaturebits_allValueGet(&featurebits);
    if (ret == ZCFG_INTERNAL_ERROR)
    {
        return ret;
    }

    mask &= 0xFFFFFFFF;
    *value = (mask & featurebits);

    return ret;
}
/*!
 *  Check if it support ADSL Annex A.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_adslAnnexAGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_ADSL_ANNEXA, value);
    return ret;
}
/*!
 *  Check if it support ADSL Annex B.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_adslAnnexBGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_ADSL_ANNEXB, value);
    return ret;
}
/*!
 *  Check if it support ADSL Annex C.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_adslAnnexCGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_ADSL_ANNEXC, value);
    return ret;
}
/*!
 *  Check if it support VDSL 17a.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_vdsl17aGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_VDSL_17A, value);
    return ret;
}
/*!
 *  Check if it support VDSL 35b.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_vdsl35bGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_VDSL_35B, value);
    return ret;
}
/*!
 *  Check if it support VDSL G.fast.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_vdslGfastGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_VDSL_GFAST, value);
    return ret;
}

/*!
 *  Check if it support DSL.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_support_DSL(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;
    unsigned int featurebits;
    unsigned int check_DSL = (ZYFEATUREBITS_ADSL_ANNEXA | ZYFEATUREBITS_ADSL_ANNEXB | ZYFEATUREBITS_ADSL_ANNEXC | ZYFEATUREBITS_VDSL_17A | ZYFEATUREBITS_VDSL_35B | ZYFEATUREBITS_VDSL_GFAST);
    ret = _zyxelfeaturebits_allValueGet(&featurebits);
    if (ret == ZCFG_INTERNAL_ERROR)
    {
        return ret;
    }
    *value = ((featurebits & check_DSL) ? 1 : 0);
    return ret;
}

/*!
 *  Check if it support Wifi 2.4G.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_wifi24GGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_WIFI_24G, value);
    return ret;
}
/*!
 *  Check if it support Wifi 2.4G FEM.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_wifi24GFemGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_WIFI_24G_FEM, value);
    return ret;
}
/*!
 *  Check if it support Wifi 2.4G external antenna.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_wifi24GExAntGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_WIFI_24G_EXT_ANT, value);
    return ret;
}
/*!
 *  Check if it support Wifi 5G.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_wifi5GGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_WIFI_5G, value);
    return ret;
}
/*!
 *  Check if it support Wifi 5G FEM.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_wifi5GFemGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_WIFI_5G_FEM, value);
    return ret;
}
/*!
 *  Check if it support Wifi 5G external antenna.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_wifi5GExAntGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_WIFI_5G_EXT_ANT, value);
    return ret;
}
/*!
 *  Check if it support VoIP.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_voipGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_VOIP, value);
    return ret;
}
/*!
 *  Check if it only support single VoIP.
 *
 *  @param[out] value       1:single voip, 0:dobule voip(default).
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_voipSinglePortGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_VOIP_SINGLE_PORT, value);
    return ret;
}
/*!
 *  Check if it support ZigBee.
 *
 *  @param[out] value       1:support, 0:not support.
 *  @return zcfgRet_t       ZCFG_SUCCESS:success(0), ZCFG_INTERNAL_ERROR:fail(-5).
 */
zcfgRet_t libzyutil_zyxelfeaturebits_zigbeeGet(int *value)
{
    zcfgRet_t ret = ZCFG_SUCCESS;

    if (value == NULL)
    {
        zcfgLog(ZCFG_LOG_ERR, "%s : value == NULL.\n", __func__);
        return ZCFG_INTERNAL_ERROR;
    }

    ret = _zyxelfeaturebits_offsetValueGet(ZYFEATUREBITS_ZIGBEE, value);
    return ret;
}
