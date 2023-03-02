tr98Parameter_t para_Root[] = {
	{ "DeviceSummary", PARAMETER_ATTR_READONLY, 1025, json_type_string},
	{ "LANDeviceNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "WANDeviceNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
#ifdef ABUU_CUSTOMIZATION_MULTIPLE_CONFIG
	{ "X_ZYXEL_PROFILE", PARAMETER_ATTR_READONLY, 8, json_type_string},
#endif
	{ NULL, 0, 0, 0}
};
