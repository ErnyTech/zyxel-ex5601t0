extern zcfgRet_t ZyEEObjGet(char *tr98FullPathName, int handler, struct json_object **tr98Jobj, bool updateFlag);
extern zcfgRet_t ZyEEObjSet(char *tr98FullPathName, int handler, struct json_object *tr98Jobj, struct json_object *multiJobj, char *paramfault);
extern zcfgRet_t ZyEEObjAdd(char *tr98FullPathName, int *idx);

tr98Parameter_t para_SoftwareMoudles[] =
{
	{ "ExecEnvNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "DeploymentUnitNumberOfEntries", PARAMETER_ATTR_READONLY, 8, json_type_uint32},
    { "ExecutionUnitNumberOfEntries", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_ZyeeExecEnvConf[] =
{
	{ "Action", PARAMETER_ATTR_WRITE, 16, json_type_string},
	{ "Name", PARAMETER_ATTR_WRITE, 32, json_type_string},
    { "AllocatedMemory", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ NULL, 0, 0, 0}
};
	
tr98Parameter_t para_ZyeeExecEnv[] =
{
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Status", PARAMETER_ATTR_READONLY, 8, json_type_string},
    { "Reset", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "Alias", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Name", PARAMETER_ATTR_READONLY, 32, json_type_string},
	{ "Type", PARAMETER_ATTR_READONLY, 64, json_type_string},
    { "InitialRunLevel", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
    { "RequestedRunLevel", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
    { "CurrentRunLevel", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "InitialExecutionUnitRunLevel", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
	{ "Vendor", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Version", PARAMETER_ATTR_READONLY, 32, json_type_string},
    { "ParentExecEnv", PARAMETER_ATTR_READONLY, 36, json_type_string},
	{ "AllocatedDiskSpace", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
	{ "AvailableDiskSpace", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "AllocatedMemory", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "AvailableMemory", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "ActiveExecutionUnits", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "ProcessorRefList", PARAMETER_ATTR_READONLY, 288, json_type_string},
    { "X_ZYXEL_OpStatus", PARAMETER_ATTR_WRITE, 0, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_ZyeeDeploymentUnit[] =
{
	{ "UUID", PARAMETER_ATTR_READONLY, 36, json_type_string},
	{ "DUID", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Alias", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Name", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Status", PARAMETER_ATTR_READONLY, 12, json_type_string},
    { "Resolved", PARAMETER_ATTR_READONLY, 0, json_type_boolean},
	{ "URL", PARAMETER_ATTR_READONLY, 1024, json_type_string},
	{ "Description", PARAMETER_ATTR_READONLY, 256, json_type_string},
    { "Vendor", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Version", PARAMETER_ATTR_READONLY, 128, json_type_string},
    { "VendorLogList", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "VendorconfigList", PARAMETER_ATTR_READONLY, 336, json_type_string},
	{ "ExecutionUnitList", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "ExecutionEnvRef", PARAMETER_ATTR_READONLY, 36, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_ZyeeExecutionUnit[] =
{
	{ "EUID", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "Alias", PARAMETER_ATTR_WRITE, 64, json_type_string},
	{ "Name", PARAMETER_ATTR_READONLY, 64, json_type_string},
	{ "ExecEnvLabel", PARAMETER_ATTR_READONLY, 64, json_type_string},
    { "Status", PARAMETER_ATTR_READONLY, 8, json_type_string},
    { "RequestedState", PARAMETER_ATTR_WRITE, 8, json_type_string},
    { "ExecutionFaultCode", PARAMETER_ATTR_READONLY, 16, json_type_string},
    { "ExecutionFaultMessage", PARAMETER_ATTR_READONLY, 256, json_type_string},
	{ "AutoStart", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
    { "RunLevel", PARAMETER_ATTR_WRITE, 0, json_type_uint32},
    { "Vendor", PARAMETER_ATTR_READONLY, 128, json_type_string},
	{ "Version", PARAMETER_ATTR_READONLY, 32, json_type_string},
    { "Description", PARAMETER_ATTR_READONLY, 256, json_type_string},
    { "DiskSpaceInUse", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "MemoryInUse", PARAMETER_ATTR_READONLY, 0, json_type_uint32},
    { "References", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "AssociatedProcessList", PARAMETER_ATTR_READONLY, 384, json_type_string},
    { "VendorLogList", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "VendorconfigList", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "SupportedDataModelList", PARAMETER_ATTR_READONLY, 336, json_type_string},
    { "ExecutionEnvRef", PARAMETER_ATTR_READONLY, 36, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_ZyeeExecutionUnitExtensions[] =
{
	{ "CmdLineStart", PARAMETER_ATTR_WRITE, 512, json_type_string},
    { "CmdLineStop", PARAMETER_ATTR_WRITE, 512, json_type_string},
	{ NULL, 0, 0, 0}
};

tr98Parameter_t para_ZyeeDUStateChangeComplPolicy[] =
{
	{ "Enable", PARAMETER_ATTR_WRITE, 0, json_type_boolean},
	{ "OperationTypeFilter", PARAMETER_ATTR_WRITE, 24, json_type_string},
	{ "ResultTypeFilter", PARAMETER_ATTR_WRITE, 7, json_type_string},
	{ "FaultCodeFilter", PARAMETER_ATTR_WRITE, 97, json_type_string},
	{ NULL, 0, 0, 0}
};