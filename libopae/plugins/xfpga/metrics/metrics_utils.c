// Copyright(c) 2017, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/**
* \file metrics_utils.c
* \brief fpga metrics API
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <uuid/uuid.h>

#include "common_int.h"
#include "metrics_int.h"
#include "types_int.h"
#include "opae/metrics.h"
#include "metrics/vector.h"

#include "metrics/bmc/bmc.h"
#include "safe_string/safe_string.h"
#include "metrics/metrics_metadata.h"
#include <dlfcn.h>

fpga_result sysfs_path_is_dir(const char *path)
{
	struct stat astats;

	if (path == NULL) {
		return FPGA_INVALID_PARAM;
	}

	if ((stat(path, &astats)) != 0) {
		printf("stat() failed =  %s \n", path);
		return FPGA_NOT_FOUND;
	}

	if (S_ISDIR(astats.st_mode)) {
		return FPGA_OK;
	}

	return FPGA_NOT_FOUND;
}


fpga_result add_metric_vector(fpga_metric_vector *vector,
								uint64_t metric_id,
								const char *qualifier_name,
								const char *group_name,
								const char *group_sysfs,
								const char *metric_name,
								const char *metric_sysfs,
								const char *metric_units,
								enum fpga_metric_datatype  metric_datatype,
								enum fpga_metric_type	metric_type,
								enum fpga_hw_type	hw_type,
								uint64_t mmio_offset)
{

	fpga_result result = FPGA_OK;
	struct _fpga_enum_metric *fpga_enum_metric = NULL;
	errno_t e = 0;

	if (vector == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	if ((group_name == NULL) ||
		(group_sysfs == NULL) ||
		(metric_name == NULL) ||
		(metric_sysfs == NULL) ||
		(qualifier_name == NULL) ||
		(metric_units == NULL)) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	fpga_enum_metric = (struct _fpga_enum_metric *)malloc(sizeof(struct _fpga_enum_metric));
	if (fpga_enum_metric == NULL) {
		FPGA_ERR("Failed to allocate memory");
		return FPGA_NO_MEMORY;
	}


	e = strncpy_s(fpga_enum_metric->group_name, sizeof(fpga_enum_metric->group_name),
		group_name, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;

	e = strncpy_s(fpga_enum_metric->group_sysfs, sizeof(fpga_enum_metric->group_sysfs),
		group_sysfs, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;


	e = strncpy_s(fpga_enum_metric->metric_name, sizeof(fpga_enum_metric->metric_name),
		metric_name, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;

	e = strncpy_s(fpga_enum_metric->metric_sysfs, sizeof(fpga_enum_metric->metric_sysfs),
		metric_sysfs, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;

	e = strncpy_s(fpga_enum_metric->qualifier_name, sizeof(fpga_enum_metric->qualifier_name),
		qualifier_name, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;


	e = strncpy_s(fpga_enum_metric->metric_units, sizeof(fpga_enum_metric->metric_units),
		metric_units, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;


	fpga_enum_metric->metric_type = metric_type;
	fpga_enum_metric->metric_datatype = metric_datatype;
	fpga_enum_metric->hw_type = hw_type;
	fpga_enum_metric->metric_id = metric_id;
	fpga_enum_metric->mmio_offset = mmio_offset;

	fpga_vector_push(vector, fpga_enum_metric);

	return result;

out_free:
	free(fpga_enum_metric);
	return FPGA_INVALID_PARAM;
}


fpga_result get_metric_data_info(const char *group_name,
								const char *metric_name,
								fpga_metric_metadata *metric_data_serach,
								uint64_t size,
								fpga_metric_metadata *metric_data)
{
	fpga_result result = FPGA_OK;
	uint64_t i = 0;

	if (group_name == NULL ||
		metric_name == NULL ||
		metric_data_serach == NULL||
		metric_data == NULL) {
		FPGA_ERR("Invlaid Input Paramters");
		return FPGA_INVALID_PARAM;
	}

	for (i = 0; i < size; i++) {


		if (((strcasecmp(metric_data_serach[i].group_name, group_name) == 0) &&
			(strcasecmp(metric_data_serach[i].metric_name, metric_name) == 0))) {
			//	printf("get_metrics_info FOUND \n");
			*metric_data = (struct fpga_metric_metadata_t)metric_data_serach[i];
			return result;
		}

	}

	return FPGA_NOT_SUPPORTED;

}


fpga_result enum_thermalmgmt_metrics(fpga_metric_vector *vector,
									uint64_t *metric_id,
									char *sysfspath,
									enum fpga_hw_type	hw_type)
{
	fpga_result result = FPGA_OK;
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	char sysfs_path[SYSFS_PATH_MAX] = { 0 };
	char metric_sysfs_path[SYSFS_PATH_MAX] = { 0 };
	fpga_metric_metadata metric_data;


	if (vector == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	if (sysfspath == NULL ||
		metric_id == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", sysfspath, THERLGMT);

	dir = opendir(sysfs_path);
	if (NULL == dir) {
		FPGA_MSG("can't find dir %s ", strerror(errno));
		return FPGA_NOT_FOUND;
	}

	while ((dirent = readdir(dir)) != NULL) {
		if (!strcmp(dirent->d_name, "."))
			continue;
		if (!strcmp(dirent->d_name, ".."))
			continue;
		if (!strcmp(dirent->d_name, REVISION))
			continue;

		snprintf_s_ss(metric_sysfs_path, sizeof(metric_sysfs_path), "%s/%s", sysfs_path, dirent->d_name);

		*metric_id = *metric_id + 1;

		result =  get_metric_data_info(THERLGMT, dirent->d_name, mcp_metric_metadata, MCP_MDATA_SIZE, &metric_data);
		if (result != FPGA_OK) {
			FPGA_MSG("Failed to get metric metadata ");
		}

		result = add_metric_vector(vector, *metric_id, THERLGMT, THERLGMT, sysfs_path, dirent->d_name, metric_sysfs_path, metric_data.metric_units,
			FPGA_METRIC_DATATYPE_INT, FPGA_METRIC_TYPE_THERMAL, hw_type,0);
		if (result != FPGA_OK) {
			FPGA_MSG("Failed to add metrics");
			return result;
		}

	}

	return result;
}


fpga_result enum_powermgmt_metrics(fpga_metric_vector *vector,
								uint64_t *metric_id,
								char *sysfspath,
								enum fpga_hw_type hw_type)
{
	fpga_result result = FPGA_OK;
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	char sysfs_path[SYSFS_PATH_MAX] = { 0 };
	char metric_sysfs_path[SYSFS_PATH_MAX] = { 0 };

	if (vector == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	if (sysfspath == NULL ||
		metric_id == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", sysfspath, PWRMGMT);

	dir = opendir(sysfs_path);
	if (NULL == dir) {
		FPGA_MSG("can't find dir %s ", strerror(errno));
		return FPGA_NOT_FOUND;
	}

	while ((dirent = readdir(dir)) != NULL) {
		if (!strcmp(dirent->d_name, "."))
			continue;
		if (!strcmp(dirent->d_name, ".."))
			continue;
		if (!strcmp(dirent->d_name, REVISION))
			continue;

		snprintf_s_ss(metric_sysfs_path, sizeof(metric_sysfs_path), "%s/%s", sysfs_path, dirent->d_name);


		*metric_id = *metric_id + 1;
		fpga_metric_metadata metric_data;
		result = get_metric_data_info(PWRMGMT, dirent->d_name, mcp_metric_metadata, MCP_MDATA_SIZE, &metric_data);
		if (result != FPGA_OK) {
			FPGA_MSG("Failed to get metric metadata ");
		}


		result = add_metric_vector(vector, *metric_id, PWRMGMT, PWRMGMT, sysfs_path, dirent->d_name, metric_sysfs_path, metric_data.metric_units,
			FPGA_METRIC_DATATYPE_INT, FPGA_METRIC_TYPE_POWER, hw_type,0);
		if (result != FPGA_OK) {
			FPGA_MSG("Failed to add metrics");
			return result;
		}

	}

	return result;
}



fpga_result enum_perf_counter_items(fpga_metric_vector *vector,
								uint64_t *metric_id,
								char *qualifier_name,
								char *sysfspath,
								char *sysfs_name,
								enum fpga_metric_type metric_type,
								enum fpga_hw_type  hw_type)
{
	fpga_result result = FPGA_OK;
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	char sysfs_path[SYSFS_PATH_MAX] = { 0 };
	char metric_sysfs_path[SYSFS_PATH_MAX] = { 0 };
	char qname[SYSFS_PATH_MAX] = { 0 };


	if (vector == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	if ((sysfspath == NULL) ||
		(sysfs_name == NULL) ||
		(qualifier_name == NULL) ||
		(metric_id == NULL) ) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}


	snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", sysfspath, sysfs_name);

	dir = opendir(sysfs_path);
	if (NULL == dir) {
		FPGA_MSG("can't find dir %s ", strerror(errno));
		return FPGA_NOT_FOUND;
	}

	while ((dirent = readdir(dir)) != NULL) {
		if (!strcmp(dirent->d_name, "."))
			continue;
		if (!strcmp(dirent->d_name, ".."))
			continue;
		if (!strcmp(dirent->d_name, PERF_ENABLE))
			continue;

		if (!strcmp(dirent->d_name, PERF_FREEZE))
			continue;

		if (dirent->d_type == DT_DIR) {

			snprintf_s_ss(qname, sizeof(qname), "%s:%s", qualifier_name, dirent->d_name);

			result = enum_perf_counter_items(vector, metric_id, qname, sysfs_path, dirent->d_name, metric_type, hw_type);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to add metrics");
			}
			continue;

		}

		snprintf_s_ss(metric_sysfs_path, sizeof(metric_sysfs_path), "%s/%s", sysfs_path, dirent->d_name);
		*metric_id = *metric_id + 1;

		result = add_metric_vector(vector, *metric_id, qualifier_name, "performance", sysfs_path, dirent->d_name,
			metric_sysfs_path,"", FPGA_METRIC_DATATYPE_INT, metric_type, hw_type,0);
		if (result != FPGA_OK) {
			FPGA_MSG("Failed to add metrics");
			return result;
		}

	}

	return result;

}

fpga_result enum_perf_counter_metrics(fpga_metric_vector *vector,
								uint64_t *metric_id,
								char *sysfspath,
								enum fpga_hw_type  hw_type)
{
	fpga_result result = FPGA_OK;
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	char sysfs_path[SYSFS_PATH_MAX] = { 0 };
	char sysfs_ipath[SYSFS_PATH_MAX] = { 0 };
	char sysfs_dpath[SYSFS_PATH_MAX] = { 0 };

	char qualifier_name[SYSFS_PATH_MAX] = { 0 };

	if (vector == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	if (sysfspath == NULL ||
		metric_id == NULL) {
		FPGA_ERR("Invlaid Input parameters");
		return FPGA_INVALID_PARAM;
	}

	snprintf_s_ss(sysfs_ipath, sizeof(sysfs_ipath), "%s/%s", sysfspath, IPERF);
	snprintf_s_ss(sysfs_dpath, sizeof(sysfs_dpath), "%s/%s", sysfspath, DPERF);


	if (sysfs_path_is_dir(sysfs_ipath) == FPGA_OK) {

		snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", sysfspath, IPERF);


	} else if (sysfs_path_is_dir(sysfs_dpath) == FPGA_OK) {

		snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", sysfspath, DPERF);

	}
	else {

		FPGA_MSG("NO Perf Counters");
		return FPGA_NOT_FOUND;
	}

	dir = opendir(sysfs_path);
	if (NULL == dir) {
		FPGA_MSG("can't find dirt %s ", strerror(errno));
		return FPGA_NOT_FOUND;
	}

	while ((dirent = readdir(dir)) != NULL) {

		if (!strcmp(dirent->d_name, "."))
			continue;
		if (!strcmp(dirent->d_name, ".."))
			continue;
		if (!strcmp(dirent->d_name, REVISION))
			continue;


		if (strcmp(dirent->d_name, PERF_CACHE) == 0) {
			snprintf_s_ss(qualifier_name, sizeof(qualifier_name), "%s:%s", PERF, PERF_CACHE);
			result = enum_perf_counter_items(vector, metric_id, qualifier_name, sysfs_path, dirent->d_name, FPGA_METRIC_TYPE_PERF_CACHE, hw_type);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to add metrics");
			}

		}

		if (strcmp(dirent->d_name, PERF_FABRIC) == 0) {
			snprintf_s_ss(qualifier_name, sizeof(qualifier_name), "%s:%s", PERF, PERF_FABRIC);
			result = enum_perf_counter_items(vector, metric_id, qualifier_name, sysfs_path, dirent->d_name, FPGA_METRIC_TYPE_PERF_FABRIC, hw_type);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to add metrics");
			}

		}

		if (strcmp(dirent->d_name, PERF_IOMMU) == 0) {
			snprintf_s_ss(qualifier_name, sizeof(qualifier_name), "%s:%s", PERF, PERF_IOMMU);
			result = enum_perf_counter_items(vector, metric_id, qualifier_name, sysfs_path, dirent->d_name, FPGA_METRIC_TYPE_PERF_IOMMU, hw_type);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to add metrics");
			}

		}

	}

	return result;
}

fpga_result xfpga_bmcLoadSDRs(struct _fpga_handle *_handle, fpga_token token, bmc_sdr_handle *records,
	uint32_t *num_sensors)
{
	fpga_result result = FPGA_NOT_FOUND;
	fpga_result(*bmcLoadSDRs)(fpga_token token, bmc_sdr_handle *records,
		uint32_t *num_sensors);

	if (_handle->dl_handle != NULL) {

		bmcLoadSDRs= dlsym(_handle->dl_handle, "bmcLoadSDRs");

		result = bmcLoadSDRs(token, records, num_sensors);

	}
	return result;
}

fpga_result xfpga_bmcDestroySDRs(struct _fpga_handle *_handle,bmc_sdr_handle *records)
{
	fpga_result result = FPGA_NOT_FOUND;
	fpga_result(*bmcDestroySDRs)(bmc_sdr_handle *records);

	if (_handle->dl_handle != NULL) {

		bmcDestroySDRs = dlsym(_handle->dl_handle, "bmcDestroySDRs");

		result = bmcDestroySDRs(records);

	}
	return result;
}


fpga_result xfpga_bmcReadSensorValues(struct _fpga_handle *_handle, bmc_sdr_handle records,
	bmc_values_handle *values,
	uint32_t *num_values)
{
	fpga_result result = FPGA_NOT_FOUND;
	fpga_result(*bmcReadSensorValues)(bmc_sdr_handle records,bmc_values_handle *values,uint32_t *num_values);

	if (_handle->dl_handle != NULL) {

		bmcReadSensorValues = dlsym(_handle->dl_handle, "bmcReadSensorValues");

		result = bmcReadSensorValues(records, values, num_values);

	}
	return result;
}


fpga_result xfpga_bmcDestroySensorValues(struct _fpga_handle *_handle, bmc_values_handle *values)
{
	fpga_result result = FPGA_NOT_FOUND;
	fpga_result(*bmcDestroySensorValues)(bmc_values_handle *values);

	if (_handle->dl_handle != NULL) {

		bmcDestroySensorValues = dlsym(_handle->dl_handle, "bmcDestroySensorValues");

		result = bmcDestroySensorValues(values);

	}
	return result;
}

fpga_result xfpga_bmcGetSensorReading(struct _fpga_handle *_handle, bmc_values_handle values,
	uint32_t sensor_number, uint32_t *is_valid,
	double *value)
{
	fpga_result result = FPGA_NOT_FOUND;
	fpga_result(*bmcGetSensorReading)(bmc_values_handle values,
		uint32_t sensor_number, uint32_t *is_valid,
		double *value);

	if (_handle->dl_handle != NULL) {

		bmcGetSensorReading = dlsym(_handle->dl_handle, "bmcGetSensorReading");

		result = bmcGetSensorReading(values, sensor_number, is_valid, value);

	}
	return result;
}

fpga_result xfpga_bmcGetSDRDetails(struct _fpga_handle *_handle, bmc_values_handle values, uint32_t sensor_number,
	sdr_details *details)
{
	fpga_result result = FPGA_NOT_FOUND;
	fpga_result(*bmcGetSDRDetails)(bmc_values_handle values, uint32_t sensor_number,
		sdr_details *details);

	if (_handle->dl_handle != NULL) {

		bmcGetSDRDetails = dlsym(_handle->dl_handle, "bmcGetSDRDetails");

		result = bmcGetSDRDetails(values, sensor_number, details);

	}
	return result;
}



fpga_result  enum_bmc_metrics_info(struct _fpga_handle *_handle,fpga_token token, fpga_metric_vector *vector, uint64_t *metric_id, enum fpga_hw_type  hw_type)
{
	fpga_result result = FPGA_OK;
	bmc_sdr_handle records;
	uint32_t num_sensors;
	bmc_values_handle values;
	uint32_t num_values;
	sdr_details details;
	uint32_t x;
	//enum fpga_metrics_datatype  metrics_datatype = FPGA_METRICS_DATATYPE_FLAOT;
	enum fpga_metric_type   metric_type = FPGA_METRIC_TYPE_POWER;
	char group_name[SYSFS_PATH_MAX] = { 0 };

	if (vector == NULL ||
		metric_id == NULL) {
		FPGA_ERR("Invalid input");
		return result;
	}
	result = xfpga_bmcLoadSDRs(_handle,token, &records, &num_sensors);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to load BMC SDR.");
		return result;
	}

	result = xfpga_bmcReadSensorValues(_handle,records, &values, &num_values);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to read BMC sensor values.");
		return result;
	}

	for (x = 0; x < num_sensors; x++) {
		result = xfpga_bmcGetSDRDetails(_handle,values, x, &details);

		*metric_id = *metric_id + 1;
		if (details.type == BMC_THERMAL)  {
			metric_type = FPGA_METRIC_TYPE_THERMAL;
		}else if (details.type == BMC_POWER){
			metric_type = FPGA_METRIC_TYPE_POWER;
		}else {

		}

		char units[SYSFS_PATH_MAX] = { 0 };
		snprintf(units, sizeof(units), "%ls", details.units);


		result = add_metric_vector(vector, *metric_id, "", group_name, "", details.name, "" , units, FPGA_METRIC_DATATYPE_DOUBLE, metric_type, hw_type,0);
		if (result != FPGA_OK) {
			FPGA_MSG("Failed to add metrics");
			return result;
		}

	}

	result = xfpga_bmcDestroySensorValues(_handle,&values);

	result = xfpga_bmcDestroySDRs(_handle,&records);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to Destroy SDR.");
		return result;
	}

	return result;
}

fpga_result delete_fpga_enum_metrics_vector(struct _fpga_handle *_handle)
{
	fpga_result result = FPGA_OK;
	uint64_t i = 0;

	if (_handle == NULL) {
		FPGA_ERR("Invalid handle ");
		return FPGA_INVALID_PARAM;
	}

	if (_handle->magic != FPGA_HANDLE_MAGIC) {
		FPGA_MSG("Invalid handle");
		return FPGA_INVALID_PARAM;
	}


	for (i = 0; i < fpga_vector_total(&(_handle->fpga_enum_metric_vector)); i++) {
		fpga_vector_delete(&(_handle->fpga_enum_metric_vector), i);
	}

	fpga_vector_free(&(_handle->fpga_enum_metric_vector));

	return result;
}

fpga_result get_fpga_object_type(fpga_handle handle, fpga_objtype *objtype)
{
	fpga_result result = FPGA_OK;
	fpga_properties prop;



	result = xfpga_fpgaGetPropertiesFromHandle(handle, &prop);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to get properties");
		return result;
	}


	result = fpgaPropertiesGetObjectType(prop, objtype);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to object type.");
		return result;
	}


	fpgaDestroyProperties(&prop);

	return result;
}

fpga_result enum_fpga_metrics(fpga_handle handle)
{
	fpga_result result = FPGA_OK;
	struct _fpga_token  *_token = NULL;
	uint64_t deviceid = 0;
	// uint64_t i						= 0;
	uint64_t metric_id = 0;

	fpga_objtype objtype;
	uint64_t mmio_offset = 0;

	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;

	if (_handle == NULL) {
		FPGA_ERR("Invalid handle ");
		return FPGA_INVALID_PARAM;
	}

	if (_handle->metric_enum_status) 
		return FPGA_OK;

	_token = (struct _fpga_token *)_handle->token;
	if (_token == NULL) {
		FPGA_ERR("Invalid token within handle");
		return FPGA_INVALID_PARAM;
	}


	result = get_fpga_object_type(handle, &objtype);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to init vector");
		return result;
	}

	// Init vector
	result = fpga_vector_init(&(_handle->fpga_enum_metric_vector));
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to init vector");
		return result;
	}

	if (objtype == FPGA_ACCELERATOR) {

		result = discover_afu_metrics_feature(handle, &mmio_offset);
		if (result != FPGA_OK) {
			FPGA_ERR("Failed to init vector");
			return result;
		}


		result = enum_afu_metrics(handle,
			&(_handle->fpga_enum_metric_vector),
			&metric_id,
			mmio_offset);
			if (result != FPGA_OK) {
				FPGA_ERR("Failed to init vector");
				return result;
			}

		// enum AFU
	} else	if (objtype == FPGA_DEVICE) {

		// enum FME

		// get fpga device id.
		result = get_fpga_deviceid(_handle, &deviceid);
		if (result != FPGA_OK) {
			FPGA_ERR("Failed to read device id.");
			return result;
		}


		switch (deviceid) {
			// MCP
		case FPGA_INTEGRATED_DEVICEID: {

			result = enum_powermgmt_metrics(&(_handle->fpga_enum_metric_vector), &metric_id, _token->sysfspath, FPGA_HW_MCP);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to Enum Power metrics.");
			}

			result = enum_thermalmgmt_metrics(&(_handle->fpga_enum_metric_vector), &metric_id, _token->sysfspath, FPGA_HW_MCP);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to Enum Thermal metrics.");
			}

			result = enum_perf_counter_metrics(&(_handle->fpga_enum_metric_vector), &metric_id, _token->sysfspath, FPGA_HW_MCP);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to Enum Perforamnce metrics.");
			}

		}
		break;

		 // DCP RC
		case FPGA_DISCRETE_DEVICEID: {

			result = enum_perf_counter_metrics(&(_handle->fpga_enum_metric_vector), &metric_id, _token->sysfspath, FPGA_HW_DCP_RC);
			if (result != FPGA_OK) {
				FPGA_MSG("Failed to Enum Perforamnce metrics.");
			}

			_handle->dl_handle = dlopen("libbmc.so", RTLD_LAZY | RTLD_LOCAL);

			if (_handle->dl_handle) {
				result = enum_bmc_metrics_info(_handle, _handle->token, &(_handle->fpga_enum_metric_vector), &metric_id, FPGA_HW_DCP_RC);
				if (result != FPGA_OK) {
					FPGA_MSG("Failed to Enum Perforamnce metrics.");
				}

			}

			

		}
		break;

		default:
			FPGA_MSG("Unknown Device ID.");
		}

	} // if Object type





	/*
	struct _fpga_enum_metric* fpga_enum_metric = NULL;
	for (uint64_t i = 0; i < fpga_vector_total(&(_handle->fpga_enum_metric_vector)); i++) {
		fpga_enum_metric = (struct _fpga_enum_metric*)	fpga_vector_get(&(_handle->fpga_enum_metric_vector), i);

		printf("metrics_qualifier_name : %s\n ", fpga_enum_metric->qualifier_name);
		printf("metrics_group_name : %s\n ", fpga_enum_metric->group_name);
		printf("metrics_group_sysfs : %s\n ", fpga_enum_metric->group_sysfs);
		printf("metrics_name : %s\n ", fpga_enum_metric->metric_name);
		printf("metrics_sysfs : %s\n ", fpga_enum_metric->metric_sysfs);
		printf("metrics_id : %ld\n ", fpga_enum_metric->metric_id);
		printf("\n");

	}

	printf("total %ld \n ", fpga_vector_total(&(_handle->fpga_enum_metric_vector)));
	*/

	_handle->metric_enum_status = true;


	return result;


}


fpga_result add_metric_info(struct _fpga_enum_metric* _enum_metrics,
							struct fpga_metric_t  *fpga_metric)
{

	fpga_result result = FPGA_OK;
	errno_t e = 0;

	if ((_enum_metrics == NULL) ||
		(fpga_metric == NULL)) {

		FPGA_ERR("Invlaid Input Paramters");
		return FPGA_INVALID_PARAM;
	}




	e = strncpy_s(fpga_metric->mertic_info.group_name, sizeof(fpga_metric->mertic_info.group_name),
		_enum_metrics->group_name, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;

	e = strncpy_s(fpga_metric->mertic_info.metric_name, sizeof(fpga_metric->mertic_info.metric_name),
		_enum_metrics->metric_name, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;

	
	e = strncpy_s(fpga_metric->mertic_info.qualifier_name, sizeof(fpga_metric->mertic_info.qualifier_name),
	_enum_metrics->qualifier_name, SYSFS_PATH_MAX);
	if (EOK != e)
	goto out_free;
	
	e = strncpy_s(fpga_metric->mertic_info.metric_units, sizeof(fpga_metric->mertic_info.metric_units),
		_enum_metrics->metric_units, SYSFS_PATH_MAX);
	if (EOK != e)
		goto out_free;


	fpga_metric->mertic_info.metric_id = _enum_metrics->metric_id;

	fpga_metric->mertic_info.metric_type = _enum_metrics->metric_type;
	fpga_metric->mertic_info.metric_datatype = _enum_metrics->metric_datatype;



	return result;

out_free:

	return FPGA_INVALID_PARAM;
}




fpga_result get_bmc_metrics_values(struct _fpga_handle *_handle,fpga_token token, struct _fpga_enum_metric* _fpga_enum_metric, struct fpga_metric_t  *fpga_metric)
{
	fpga_result result = FPGA_OK;
	bmc_sdr_handle records;
	uint32_t num_sensors;
	bmc_values_handle values;
	uint32_t num_values;
	sdr_details details;
	uint32_t x;


	printf("--------------------get_bmc_info \n");
	uint32_t is_valid;
	double tmp;
	UNUSED_PARAM(_fpga_enum_metric);
	UNUSED_PARAM(fpga_metric);

	result = xfpga_bmcLoadSDRs(_handle,token, &records, &num_sensors);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to load BMC SDR.");
		return result;
	}


	result = xfpga_bmcReadSensorValues(_handle,records, &values, &num_values);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to read BMC sensor values.");
		return result;
	}

	for (x = 0; x < num_sensors; x++) {



		result = xfpga_bmcGetSDRDetails(_handle,values, x, &details);

		if ((strcasecmp(_fpga_enum_metric->metric_name, details.name) == 0) ) {

			result = xfpga_bmcGetSensorReading(_handle,values, x, &is_valid, &tmp);

			if (result != FPGA_OK) {
				continue;
			}
			if (!is_valid) {
				continue;
			}

		
			add_metric_info(_fpga_enum_metric, fpga_metric);
			fpga_metric->value.dvalue = tmp;

		}

	}


	result = xfpga_bmcDestroySensorValues(_handle,&values);

	result = xfpga_bmcDestroySDRs(_handle,&records);
	if (result != FPGA_OK) {
		FPGA_ERR("Failed to Destroy SDR.");
		return result;
	}

	return result;
}


fpga_result  get_metric_value_byid(struct _fpga_handle *_handle,fpga_metric_vector* enum_vector,
								uint64_t metric_id,
								struct fpga_metric_t  *fpga_metric)
{
	fpga_result result = FPGA_OK;
	char sysfs_path[SYSFS_PATH_MAX] = { 0 };
	uint64_t val = 0;
	uint64_t index = 0;
	metric_value value;

	if (enum_vector == NULL) {
		FPGA_ERR("Invlaid Input Paramters");
		return FPGA_INVALID_PARAM;
	}

	if (fpga_metric == NULL) {
		FPGA_ERR("Invlaid Input Paramters");
		return FPGA_INVALID_PARAM;
	}


	struct _fpga_enum_metric* _fpga_enum_metric = NULL;
	for (index = 0; index < fpga_vector_total(enum_vector); index++) {
		_fpga_enum_metric = (struct _fpga_enum_metric*)	fpga_vector_get(enum_vector, index);
		/*
		printf("metrics_qualifier_name : %s\n ", _fpga_enum_metric->qualifier_name);
		printf("metrics_group_name : %s\n ", _fpga_enum_metric->group_name);
		printf("metrics_group_sysfs : %s\n ", _fpga_enum_metric->group_sysfs);
		printf("metrics_name : %s\n ", _fpga_enum_metric->metric_name);
		printf("metrics_sysfs : %s\n ", _fpga_enum_metric->metric_sysfs);
		printf("metrics_id : %ld\n ", _fpga_enum_metric->metric_id);
		printf("--------------_fpga_enum_metric->metric_type : %d\n ", _fpga_enum_metric->metric_type);

		printf("\n");
		*/
		
	
		if (metric_id == _fpga_enum_metric->metric_id) {

			printf("FOUND metrics_id : %ld\n ", _fpga_enum_metric->metric_id);


			if ((_fpga_enum_metric->hw_type == FPGA_HW_DCP_RC) && (
				(_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_POWER) ||
				(_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_THERMAL) ))
			{
				get_bmc_metrics_values(_handle,_handle->token, _fpga_enum_metric, fpga_metric);
				continue;
			 }

			// START

			if (_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_PERF_CACHE) {

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_FREEZE);

				// Read Cache Freeze
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf cache");
					//return result;
				}

				if (val != 0x1) {
					// Write Cache Freeze
					result = sysfs_write_u64(sysfs_path, 0x1);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to clear port errors");
						//return result;
					}

				}

			} // end cache

			if (_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_PERF_IOMMU) {

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_FREEZE);

				// Read IOMMU Freeze
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf iommu");
					//return result;
				}

				if (val != 0x1) {
					// Writer IOMMU Freeze
					result = sysfs_write_u64(sysfs_path, 0x1);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to write perf iommu");
						//return result;
					}

				}

			} // end iommu

			if (_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_PERF_FABRIC) {

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_ENABLE);

				// Read Fabric Enable
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf fabric enable");
					//return result;
				}

				if (val != 0x1) {
					// Writer Fabric Enable
					result = sysfs_write_u64(sysfs_path, 0x1);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to read perf fabric enable");
						//return result;
					}

				}

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_FREEZE);

				// Read Fabric Freeze
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf fabric freeze");
					//return result;
				}

				if (val != 0x1) {
					// Write Fabric Freeze
					result = sysfs_write_u64(sysfs_path, 0x1);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to write perf fabric freeze");
						//return result;
					}

				}

			} // end fabric


			result = sysfs_read_u64(_fpga_enum_metric->metric_sysfs, &value.ivalue);
			if (result != FPGA_OK) {
				FPGA_ERR("Failed to read Metrics values");
				return result;
			}


			if (strcasecmp(_fpga_enum_metric->metric_name, "fpga_limit") == 0)  {
				value.ivalue = value.ivalue / 8;
			}

			if (strcasecmp(_fpga_enum_metric->metric_name, "xeon_limit") == 0)  {
				value.ivalue = value.ivalue / 8;
			}



			if (_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_PERF_CACHE) {

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_FREEZE);

				// Read Cache Freeze
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf cache");
					//return result;
				}

				if (val != 0x0) {
					// Write Cache Freeze
					result = sysfs_write_u64(sysfs_path, 0x0);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to write perf cache");
						//return result;
					}

				}

			} // end cache


			if (_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_PERF_IOMMU) {

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_FREEZE);

				// Read IOMMU Freeze
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf iommu");
					//return result;
				}

				if (val != 0x0) {
					// Writer IOMMU Freeze
					result = sysfs_write_u64(sysfs_path, 0x0);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to write perf iommu");
						//return result;
					}

				}

			} // end iommu


			if (_fpga_enum_metric->metric_type == FPGA_METRIC_TYPE_PERF_FABRIC) {

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_ENABLE);

				// Read Fabric Enable
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf fabric enable");
					//return result;
				}

				snprintf_s_ss(sysfs_path, sizeof(sysfs_path), "%s/%s", _fpga_enum_metric->group_sysfs, PERF_FREEZE);

				// Read Fabric Freeze
				result = sysfs_read_u64(sysfs_path, &val);
				if (result != FPGA_OK) {
					FPGA_ERR("Failed to read perf fabric freeze");
					//return result;
				}

				if (val != 0x0) {
					// Write Fabric Freeze
					result = sysfs_write_u64(sysfs_path, 0x0);
					if (result != FPGA_OK) {
						FPGA_ERR("Failed to write perf fabric freeze");
						//return result;
					}

				}

			} // end fabric

			

			printf("&&&& %-20s       | %-40s     | %-40s     |%-40ld      \n", _fpga_enum_metric->group_name,
				_fpga_enum_metric->metric_name, _fpga_enum_metric->qualifier_name,
				value.ivalue);
				
				
			// copy 

			add_metric_info(_fpga_enum_metric, fpga_metric);
			fpga_metric->value = value;

			// END

			break;
		}
	}








	return result;
}




fpga_result  get_metricid_from_serach_string(const char *serach_string, fpga_metric_vector *fpga_enum_metrics_vecotr,
	uint64_t *metric_id)
{


	fpga_result result = FPGA_OK;
	size_t init_size = 0;
	char *str = NULL;
	char *str_last = NULL;
	uint64_t i = 0;
	//errno_t e					= 0;
	struct _fpga_enum_metric* fpga_enum_metric = NULL;
	char qualifier_name[256] = { 0 };
	char metrics_name[256] = { 0 };

	//printf("ParseMetricsSearchString ENTER \n");

	if (serach_string == NULL ||
		fpga_enum_metrics_vecotr == NULL ||
		metric_id == NULL) {
		FPGA_ERR("Invlaid Input Paramters");
		return FPGA_INVALID_PARAM;
	}

	str = strrchr(serach_string, ':');
	if (str == NULL) {
		FPGA_ERR("Invlaid Input Paramters");
		return FPGA_INVALID_PARAM;
	}

	/*
	e = strncpy_s(fpga_enum_metrics->metrics_group_name, sizeof(fpga_enum_metrics->metrics_group_name),
	metrics_group_name, SYSFS_PATH_MAX);*/


	strncpy(metrics_name, str + 1, strlen(str + 1));



	init_size = strcspn(serach_string, ":");


	str_last = strrchr(serach_string, ':');



	init_size = strlen(serach_string) - strlen(str_last) + 1;
	



	strncpy(qualifier_name, serach_string, init_size);



	qualifier_name[init_size-1] = '\0';

	//printf("serach_string-> %s\n", serach_string);
	//printf("metrics_name-> %s\n", metrics_name);
	//printf("qualifier_name-> %s\n", qualifier_name);



	for (i = 0; i < fpga_vector_total(fpga_enum_metrics_vecotr); i++) {
		fpga_enum_metric = (struct _fpga_enum_metric*)	fpga_vector_get(fpga_enum_metrics_vecotr, i);

	
		if ((strcasecmp(fpga_enum_metric->qualifier_name, qualifier_name) == 0) &&
			(strcasecmp(fpga_enum_metric->metric_name, metrics_name) == 0)) {

		

			*metric_id = fpga_enum_metric->metric_id;
		}

	} // end of for loop

	return result;
}


