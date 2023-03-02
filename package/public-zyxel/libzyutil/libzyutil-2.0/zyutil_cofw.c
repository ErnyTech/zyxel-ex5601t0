#include <stdio.h>

#include "zyutil_cofw.h"
#include "zyutil.h"

#define MAX_MODEL_NAME_LEN 64

/*!
 *  Get Model Name
 *
 *  @return     model_name_t -  model name enum value
 *              ex. MODEL_VMG1312_T20A
 *
 */
model_name_t zyutil_cofw_modelNameGet(void)
{
	char modelName[MAX_MODEL_NAME_LEN] = {0};

	if (zyUtilIGetProductName(modelName) == ZCFG_SUCCESS)
	{
		if (strstr(modelName,"VMG3312-T20A") != NULL){
			return MODEL_VMG3312_T20A;
		}else if (strstr(modelName,"VMG1312-T20A") != NULL){
		   	return MODEL_VMG1312_T20A;
		}else if (strstr(modelName,"VMG8623-T50A") != NULL){
		   	return MODEL_VMG8623_T50A;
		}else if (strstr(modelName,"VMG3625-T50A") != NULL){
		   	return MODEL_VMG3625_T50A;
		}else if(strstr(modelName, "VMG8825-T50")) {
			return MODEL_VMG8825_T50;
		}else if(strstr(modelName, "VMG3927-T50K")) {
			return MODEL_VMG3927_T50K;
		}else if(strstr(modelName, "EMG5723-T50K")) {
			return MODEL_EMG5723_T50K;
		}else if (strstr(modelName,"VMG8623-T50B") != NULL){
		   	return MODEL_VMG8623_T50B;
		}else if (strstr(modelName,"EMG5523-T50A") != NULL){
		   	return MODEL_EMG5523_T50A;
		}else if (strstr(modelName,"VMG3927-B50B") != NULL){
			return MODEL_VMG3927_B50B;
		}else if (strstr(modelName,"EMG8726-B10A") != NULL){
			return MODEL_EMG8726_B10A;
		}else if (strstr(modelName,"VMG4927-B50A") != NULL){
			return MODEL_VMG4927_B50A;
		}else if (strstr(modelName,"VMG9827-B50A") != NULL){
			return MODEL_VMG9827_B50A;
		}else if (strstr(modelName,"EMG6726-B10A") != NULL){
			return MODEL_EMG6726_B10A;
		}else if (strstr(modelName,"PX7501-B0") != NULL || strstr(modelName,"PX7501-B1") != NULL){
			return MODEL_PX7501_B0;
		}else if (strstr(modelName,"AX7501-B0") != NULL || strstr(modelName,"AX7501-B1") != NULL){
			return MODEL_AX7501_B0;
		}else if (strstr(modelName,"PMG5817-T20A") != NULL){
		   	return MODEL_PMG5817_T20A;
		}else if (strstr(modelName,"PMG5917-T20A") != NULL){
		   	return MODEL_PMG5917_T20A;
		}else if (strstr(modelName,"VMG8825-B50B") != NULL){
			return MODEL_VMG8825_B50B;
		}else if (strstr(modelName,"AM3100-B0") != NULL){
			return MODEL_AM3100_B0;
		}else if (strstr(modelName,"DX5301-B0") != NULL){
			return MODEL_DX5301_B0;
		}else if (strstr(modelName,"EX5301-B0") != NULL){
			return MODEL_EX5301_B0;
		}else if (strstr(modelName,"DX5301-B1") != NULL){
			return MODEL_DX5301_B1;
		}else if (strstr(modelName,"EX5301-B1") != NULL){
			return MODEL_EX5301_B1;
		}else if (strstr(modelName,"DX5301-B2") != NULL){
			return MODEL_DX5301_B2;
		}else if (strstr(modelName,"EX5300-B3") != NULL){
			return MODEL_EX5300_B3;
		/*}else if (strstr(modelName,"EX5301-B2") != NULL){
			return MODEL_EX5301_B2;*/
		}else if (strstr(modelName,"DX5301-B3") != NULL){
			return MODEL_DX5301_B3;
		}else if (strstr(modelName,"EX5301-B3") != NULL){
			return MODEL_EX5301_B3;
		}else if (strstr(modelName,"DX3301-T0") != NULL){
		   	return MODEL_DX3301_T0;
		}else if (strstr(modelName,"EX3301-T0") != NULL){
			return MODEL_EX3301_T0;
		}else if (strstr(modelName,"DX3300-T0") != NULL){
			return MODEL_DX3300_T0;
		}else if (strstr(modelName,"EX3300-T0") != NULL){
			return MODEL_EX3300_T0;
		}else if (strstr(modelName,"EX3200-T0") != NULL){
			return MODEL_EX3200_T0;
		}else if (strstr(modelName, "DX5401-B0") != NULL) {
			return MODEL_DX5401_B0;
		}else if (strstr(modelName, "EX5401-B0") != NULL) {
			return MODEL_EX5401_B0;
		}
		else if (strstr(modelName, "PM3100-T0") != NULL) {
			return MODEL_PM3100_T0;
		}
	}

	return MODEL_UNDEFINED;
}
