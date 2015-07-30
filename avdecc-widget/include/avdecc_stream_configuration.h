/*
* Copyright (c) 2015 AudioScience Inc.
*/

/**
* stream_configuration_details.h
*
*/

#include <stdint.h>
#include <iostream>
#include <vector>
#include <wx/string.h>
#include <assert.h>
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
#include <stream_input_descriptor.h>
#include <stream_output_descriptor.h>
#include <clock_source_descriptor.h>
#include <strings_descriptor.h>
#include <stream_port_input_descriptor.h>
#include <stream_port_output_descriptor.h>
#include <audio_cluster_descriptor.h>
#include <audio_map_descriptor.h>
#include <clock_domain_descriptor.h>
#include <enumeration.h>
#include <util.h>


struct avdecc_stream_configuration_details {
	wxString stream_name;
	unsigned int channel_count;
	bool clk_sync_src_flag;
};

struct audio_mapping {
	uint16_t stream_index;
	uint16_t stream_channel;
	uint16_t cluster_offset;
	uint16_t cluster_channel;
};

class avdecc_stream_configuration
{
public:
	avdecc_stream_configuration(avdecc_lib::end_station * end_station, avdecc_lib::system *sys);
	virtual ~avdecc_stream_configuration();

	std::vector<struct avdecc_stream_configuration_details> avdecc_input_stream_config;
	std::vector<struct avdecc_stream_configuration_details> dialog_input_stream_config;
	std::vector<struct avdecc_stream_configuration_details> avdecc_output_stream_config;
	std::vector<struct avdecc_stream_configuration_details> dialog_output_stream_config;

	std::vector <struct audio_mapping> avdecc_stream_port_input_audio_mappings;
	std::vector <struct audio_mapping> dialog_stream_port_input_audio_mappings;
	std::vector <struct audio_mapping> avdecc_stream_port_output_audio_mappings;
	std::vector <struct audio_mapping> dialog_stream_port_output_audio_mappings;

	size_t get_stream_input_count(){ return m_stream_input_count; }
	size_t get_stream_output_count(){ return m_stream_output_count; }
	size_t get_stream_input_cluster_count(){ return m_stream_input_cluster_count; }
	size_t get_stream_output_cluster_count(){ return m_stream_output_cluster_count; }
	size_t get_avdecc_input_maps_count(){ return avdecc_stream_port_input_audio_mappings.size(); }
	size_t get_avdecc_output_maps_count(){ return avdecc_stream_port_output_audio_mappings.size(); }
	size_t get_dialog_input_maps_count(){ return dialog_stream_port_input_audio_mappings.size(); }
	size_t get_dialog_output_maps_count(){ return dialog_stream_port_output_audio_mappings.size(); }

	int get_avdecc_stream_input_details_by_index(unsigned int index, struct avdecc_stream_configuration_details &stream_details);
	int get_avdecc_stream_output_details_by_index(unsigned int index, struct avdecc_stream_configuration_details &stream_details);
	int get_dialog_stream_input_details_by_index(unsigned int index, struct avdecc_stream_configuration_details &stream_details);
	int get_dialog_stream_output_details_by_index(unsigned int index, struct avdecc_stream_configuration_details &stream_details);

	void set_input_output_cluster_counts(size_t input_cluster_count, size_t output_cluster_count)
	{
		m_stream_input_cluster_count = input_cluster_count;
		m_stream_output_cluster_count = output_cluster_count;
	}

	int SetStreamInfo();
	int SetAudioMappings();

private:
	avdecc_lib::end_station * m_end_station;
	avdecc_lib::system * m_sys;
	intptr_t notification_id;

	size_t m_stream_input_count;
	size_t m_stream_output_count;
	size_t m_stream_input_cluster_count;
	size_t m_stream_output_cluster_count;
	unsigned int m_input_maps_count;
	unsigned int m_output_maps_count;

	int GetStreamInfo();
	int GetAudioMappings();

	uint64_t channel_count_and_sample_rate_to_stream_format(unsigned int channel_count, uint32_t sampling_rate);
	int cmd_set_stream_format(wxString desc_name, uint16_t desc_index, uint64_t stream_format_value);
	int cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name);
	int cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity);
	int add_audio_mappings(uint16_t desc_type);
	int remove_audio_mappings(uint16_t desc_type);

	int get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
		avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
	int get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
		avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration);
	uint32_t get_next_notification_id();
};

enum
{
	NO_STRING = 65535
};
