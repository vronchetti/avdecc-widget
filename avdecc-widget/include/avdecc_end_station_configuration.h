/*
* Copyright (c) 2015 AudioScience Inc.
*/

/**
* avdecc_end_station_configuration.h
*
*/

#include <wx/string.h>
//avdecc-lib necessary headers
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <limits.h>
#include <string.h>
#include <fstream>
#include <inttypes.h>

#include <end_station.h>
#include <system.h>
#include <entity_descriptor.h>
#include <configuration_descriptor.h>
#include <audio_unit_descriptor.h>
#include <clock_source_descriptor.h>
#include <strings_descriptor.h>
#include <clock_domain_descriptor.h>
#include <enumeration.h>
#include <util.h>

class avdecc_end_station_configuration
{
public:
	avdecc_end_station_configuration(avdecc_lib::end_station * end_station, avdecc_lib::system *sys);
	virtual ~avdecc_end_station_configuration();

	std::vector<wxString> clock_source_descriptions;

	wxString get_entity_name(){ return name; }
	wxString get_entity_id(){ return entity_id; }
	wxString get_default_name(){ return default_name; }
	wxString get_mac(){ return mac; }
	wxString get_fw_ver(){ return fw_ver; }
	uint32_t get_sample_rate(){ return sample_rate; }
	uint16_t get_clock_source(){ return current_clock_source; }
	int set_sample_rate(uint32_t sampling_rate);
	int set_clock_source(uint16_t clock_source_index);
	int set_entity_name(wxString entity_name);

	int SetEntityName();
	int SetSamplingRate();
	int SetClockSource();

private:
	avdecc_lib::end_station * m_end_station;
	avdecc_lib::system * m_sys;
	intptr_t notification_id;

	wxString name;
	wxString dialog_name;
	wxString entity_id;
	wxString default_name;
	wxString mac;
	wxString fw_ver;
	uint32_t sample_rate;
	uint32_t dialog_sample_rate;
	uint16_t current_clock_source;
	uint16_t dialog_clock_source;

	int GetEndStationDetails();
	int cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name);
	int cmd_set_sampling_rate(uint32_t new_sampling_rate);
	int cmd_set_clock_source(uint16_t new_clk_src_index);
	int cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity);
	int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
		avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
	int get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
		avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
	uint32_t get_next_notification_id();
};