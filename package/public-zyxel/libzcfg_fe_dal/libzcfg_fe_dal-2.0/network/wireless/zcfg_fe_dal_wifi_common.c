#include <json/json.h>

#include "zcfg_common.h"
#include "zcfg_rdm_oid.h"
#include "zcfg_rdm_obj.h"
#include "zcfg_debug.h"
#include "zcfg_fe_rdm_string.h"
#include "zcfg_fe_dal_common.h"

#if 0
#ifdef ZYXEL_ONESSID
void zcfgDalWlanGetMainSsidIndex(int *main2gFound, int *main5gFound)
{
 	char *str;
 	struct json_object *obj = NULL;
 	objIndex_t iid;

	IID_INIT(iid);
	while(zcfgFeObjJsonGetNextWithoutUpdate(RDM_OID_WIFI_SSID, &iid, &obj) == ZCFG_SUCCESS) {
		if (json_object_get_int(json_object_object_get(obj, "X_ZYXEL_MainSSID")) == true) {
			str = (char*)json_object_get_string(json_object_object_get(obj, "LowerLayers"));
			if (strncmp(str, "WiFi.Radio.1", strlen("WiFi.Radio.1")) == 0) {
				*main2gFound = iid.idx[0];
			}
			else if (strncmp(str, "WiFi.Radio.2", strlen("WiFi.Radio.2")) == 0) {
				*main5gFound = iid.idx[0];
			}
		}
		json_object_put(obj);
		obj = NULL;
	}
	return;
}

static int oneSsidEnabled()
{
	struct json_object *oneSsidObj = NULL;
	objIndex_t oneSsidIid = {0};
	bool oneSsidEnabled = false;

	IID_INIT(oneSsidIid);

	if (zcfgFeObjJsonGet(RDM_OID_WIFI_ONE_SSID, &oneSsidIid, &oneSsidObj) == ZCFG_SUCCESS) {
		oneSsidEnabled = json_object_get_int(json_object_object_get(oneSsidObj, "Enable"));
		zcfgFeJsonObjFree(oneSsidObj);
		return (oneSsidEnabled) ? (true) : (false);
	}
	return false;
}

#define SSID_COPY_START	1
#define SSID_COPY_END	2
zcfgRet_t zcfgDalWlanCopy2gto5g()
{
	struct json_object *ssidObj = NULL, *ssidObj5g = NULL, *apObj = NULL, *apObj5g = NULL, *apSecObj = NULL, *apSecObj5g = NULL;
	objIndex_t ssidObjIid = {0}, ssidObj5gIid = {0}, apObjIid = {0}, apObj5gIid = {0}, apSecObjIid = {0}, apSecObj5gIid = {0};
	zcfgRet_t ret = ZCFG_SUCCESS;
	int main2gFound = 0, main5gFound = 0, i;
	char *str2g, *str5g;

	if (oneSsidEnabled() == false) {
		return ret;
	}
	zcfgDalWlanGetMainSsidIndex(&main2gFound, &main5gFound);

	for (i = SSID_COPY_START ; i <= SSID_COPY_END ; i++) {
		/* ssidObj */
		IID_INIT(ssidObjIid);
		ssidObjIid.level = 1;
		ssidObjIid.idx[0] = i;

		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidObjIid, &ssidObj)) != ZCFG_SUCCESS) {
			goto error;
		}
		IID_INIT(ssidObj5gIid);
		ssidObj5gIid.idx[0] = ssidObjIid.idx[0] - main2gFound + main5gFound;
		ssidObj5gIid.level = 1;
		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_SSID, &ssidObj5gIid, &ssidObj5g)) != ZCFG_SUCCESS) {
			goto error;
		}

		replaceParam(ssidObj5g, "SSID", ssidObj, "SSID");

		zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_SSID, &ssidObj5gIid, ssidObj5g, NULL);
		json_object_put(ssidObj);
		ssidObj = NULL;
		json_object_put(ssidObj5g);
		ssidObj5g = NULL;


		/* apObj */
		IID_INIT(apObjIid);
		apObjIid.level = 1;
		apObjIid.idx[0] = i;

		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &apObjIid, &apObj)) != ZCFG_SUCCESS) {
			goto error;
		}
		IID_INIT(apObj5gIid);
		apObj5gIid.idx[0] = apObjIid.idx[0] - main2gFound + main5gFound;
		apObj5gIid.level = 1;
		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT, &apObj5gIid, &apObj5g)) != ZCFG_SUCCESS) {
			goto error;
		}

		replaceParam(apObj5g, "SSIDAdvertisementEnabled", apObj, "SSIDAdvertisementEnabled");
		replaceParam(apObj5g, "IsolationEnable", apObj, "IsolationEnable");
		replaceParam(apObj5g, "X_ZYXEL_BssIdIsolation", apObj, "X_ZYXEL_BssIdIsolation");
		zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT, &apObj5gIid, apObj5g, NULL);
		json_object_put(apObj);
		apObj = NULL;
		json_object_put(apObj5g);
		apObj5g = NULL;


		/* apSecObj */
		IID_INIT(apSecObjIid);
		apSecObjIid.level = 1;
		apSecObjIid.idx[0] = i;

		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &apSecObjIid, &apSecObj)) != ZCFG_SUCCESS) {
			goto error;
		}
		IID_INIT(apSecObj5gIid);
		apSecObj5gIid.idx[0] = apSecObjIid.idx[0] - main2gFound + main5gFound;
		apSecObj5gIid.level = 1;
		if ((ret = zcfgFeObjJsonGetWithoutUpdate(RDM_OID_WIFI_ACCESS_POINT_SEC, &apSecObj5gIid, &apSecObj5g)) != ZCFG_SUCCESS) {
			goto error;
		}

		/*
		*  special case: 2.4G security mode does not support on 5G
		*       "ModesSupported": "None,WPA-Personal,WPA2-Personal,WPA-WPA2-Personal,"
		*       "ModeEnabled": "WEP-128"
		*/
		str2g = (char*)json_object_get_string(json_object_object_get(apSecObj, "ModeEnabled"));
		str5g = (char*)json_object_get_string(json_object_object_get(apSecObj5g, "ModesSupported"));
		if (strstr(str5g, str2g) != NULL) {
			replaceParam(apSecObj5g, "ModeEnabled", apSecObj, "ModeEnabled");
			replaceParam(apSecObj5g, "WEPKey", apSecObj, "WEPKey");
			replaceParam(apSecObj5g, "X_ZYXEL_WEPKey2", apSecObj, "X_ZYXEL_WEPKey2");
			replaceParam(apSecObj5g, "X_ZYXEL_WEPKey3", apSecObj, "X_ZYXEL_WEPKey3");
			replaceParam(apSecObj5g, "X_ZYXEL_WEPKey4", apSecObj, "X_ZYXEL_WEPKey4");
			replaceParam(apSecObj5g, "X_ZYXEL_WEP64Key1", apSecObj, "X_ZYXEL_WEP64Key1");
			replaceParam(apSecObj5g, "X_ZYXEL_WEP64Key2", apSecObj, "X_ZYXEL_WEP64Key2");
			replaceParam(apSecObj5g, "X_ZYXEL_WEP64Key3", apSecObj, "X_ZYXEL_WEP64Key3");
			replaceParam(apSecObj5g, "X_ZYXEL_WEP64Key4", apSecObj, "X_ZYXEL_WEP64Key4");
			replaceParam(apSecObj5g, "X_ZYXEL_DefaultKeyID", apSecObj, "X_ZYXEL_DefaultKeyID");
			replaceParam(apSecObj5g, "X_ZYXEL_WepModeSelect", apSecObj, "X_ZYXEL_WepModeSelect");
			replaceParam(apSecObj5g, "PreSharedKey", apSecObj, "PreSharedKey");
			replaceParam(apSecObj5g, "KeyPassphrase", apSecObj, "KeyPassphrase");
			replaceParam(apSecObj5g, "RekeyingInterval", apSecObj, "RekeyingInterval");
			replaceParam(apSecObj5g, "X_ZYXEL_AutoGenPSK", apSecObj, "X_ZYXEL_AutoGenPSK");
			replaceParam(apSecObj5g, "X_ZYXEL_Auto_PSK", apSecObj, "X_ZYXEL_Auto_PSK");
			replaceParam(apSecObj5g, "X_ZYXEL_Auto_PSK_Length", apSecObj, "X_ZYXEL_Auto_PSK_Length");
			replaceParam(apSecObj5g, "X_ZYXEL_AutoGenPSK_algorithm", apSecObj, "X_ZYXEL_AutoGenPSK_algorithm");
			replaceParam(apSecObj5g, "X_ZYXEL_AutoGenWepKey", apSecObj, "X_ZYXEL_AutoGenWepKey");
			replaceParam(apSecObj5g, "X_ZYXEL_Auto_WepKey26", apSecObj, "X_ZYXEL_Auto_WepKey26");
			replaceParam(apSecObj5g, "X_ZYXEL_Auto_WepKey10", apSecObj, "X_ZYXEL_Auto_WepKey10");
			replaceParam(apSecObj5g, "X_ZYXEL_PassphraseType", apSecObj, "X_ZYXEL_PassphraseType");
			replaceParam(apSecObj5g, "X_ZYXEL_WEPAuthentication", apSecObj, "X_ZYXEL_WEPAuthentication");
			replaceParam(apSecObj5g, "X_ZYXEL_WEPEncryption", apSecObj, "X_ZYXEL_WEPEncryption");
			replaceParam(apSecObj5g, "X_ZYXEL_WPAAuthentication", apSecObj, "X_ZYXEL_WPAAuthentication");
			replaceParam(apSecObj5g, "X_ZYXEL_11iAuthentication", apSecObj, "X_ZYXEL_11iAuthentication");
			replaceParam(apSecObj5g, "X_ZYXEL_Encryption", apSecObj, "X_ZYXEL_Encryption");
			replaceParam(apSecObj5g, "X_ZYXEL_WPAEncryption", apSecObj, "X_ZYXEL_WPAEncryption");
			replaceParam(apSecObj5g, "X_ZYXEL_11iEncryption", apSecObj, "X_ZYXEL_11iEncryption");
			replaceParam(apSecObj5g, "X_ZYXEL_Preauth", apSecObj, "X_ZYXEL_Preauth");
			replaceParam(apSecObj5g, "X_ZYXEL_ReauthInterval", apSecObj, "X_ZYXEL_ReauthInterval");

			if (i == SSID_COPY_END) {
				zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_ACCESS_POINT_SEC, &apSecObj5gIid, apSecObj5g, NULL);
			}
			else {
				zcfgFeObjJsonSetWithoutApply(RDM_OID_WIFI_ACCESS_POINT_SEC, &apSecObj5gIid, apSecObj5g, NULL);
			}
		}
		json_object_put(apSecObj);
		apSecObj = NULL;
		json_object_put(apSecObj5g);
		apSecObj5g = NULL;
	}

error:
    json_object_put(ssidObj);
    json_object_put(ssidObj5g);
    json_object_put(apObj);
    json_object_put(apObj5g);
    json_object_put(apSecObj);
    json_object_put(apSecObj5g);

    return ret;
}

int enableOneSsid()
{
    struct json_object *oneSsidObj = NULL;
    objIndex_t oneSsidIid = {0};
    bool oneSsidEnabled = true;
	zcfgRet_t ret = ZCFG_SUCCESS;

    IID_INIT(oneSsidIid);

    if (zcfgFeObjJsonGet(RDM_OID_WIFI_ONE_SSID, &oneSsidIid, &oneSsidObj) == ZCFG_SUCCESS) {
		json_object_object_add(oneSsidObj, "Enable", json_object_new_boolean(oneSsidEnabled));
		zcfgFeObjJsonBlockedSet(RDM_OID_WIFI_ONE_SSID, &oneSsidIid, oneSsidObj, NULL);
        zcfgFeJsonObjFree(oneSsidObj);
		ret = ZCFG_INTERNAL_ERROR;
    }
	return ret;
}

#endif

#endif