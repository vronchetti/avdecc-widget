/*
 * Licensed under the MIT License (MIT)
 *
 * Copyright (c) 2015 AudioScience Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * stream_configuration.cpp
 *
 */

#include <iostream>
#include "stream_configuration.h"


stream_configuration::stream_configuration(avdecc_lib::end_station * end_station, avdecc_lib::system * sys)
{
    m_end_station = end_station;
    m_sys = sys;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);
    
    m_stream_input_count = configuration->stream_input_desc_count();
    m_stream_output_count = configuration->stream_output_desc_count();
    
    GetStreamInfo();
    GetAudioMappings();
}
stream_configuration::~stream_configuration() {}

size_t stream_configuration::get_stream_input_count()
{
    return m_stream_input_count;
}

size_t stream_configuration::get_stream_output_count()
{
    return m_stream_output_count;
}

size_t stream_configuration::get_stream_input_cluster_count()
{
    return m_stream_input_cluster_count;
}

size_t stream_configuration::get_stream_output_cluster_count()
{
    return m_stream_output_cluster_count;
}

void stream_configuration::set_input_output_cluster_counts(size_t input_cluster_count, size_t output_cluster_count)
{
    m_stream_input_cluster_count = input_cluster_count;
    m_stream_output_cluster_count = output_cluster_count;
}

size_t stream_configuration::get_avdecc_input_maps_count()
{
    return avdecc_stream_port_input_audio_mappings.size();
}

size_t stream_configuration::get_avdecc_output_maps_count()
{
    return avdecc_stream_port_output_audio_mappings.size();
}

size_t stream_configuration::get_dialog_input_maps_count()
{
    return dialog_stream_port_input_audio_mappings.size();
}

size_t stream_configuration::get_dialog_output_maps_count()
{
    return dialog_stream_port_output_audio_mappings.size();
}

int stream_configuration::get_avdecc_stream_input_details_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_input_count)
        return -1;
    
    stream_details = avdecc_input_stream_config.at(index);
    return 0;
}

int stream_configuration::get_avdecc_stream_output_details_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_output_count)
        return -1;
    
    stream_details = avdecc_output_stream_config.at(index);
    return 0;
}

int stream_configuration::get_dialog_stream_input_details_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_input_count)
        return -1;
    
    stream_details = dialog_input_stream_config.at(index);
    return 0;
}

int stream_configuration::get_dialog_stream_output_details_by_index(unsigned int index, struct stream_configuration_details &stream_details)
{
    if (index >= m_stream_output_count)
        return -1;
    
    stream_details = dialog_output_stream_config.at(index);
    return 0;
}

int stream_configuration::GetStreamInfo()
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);

    /****************** Stream Input Info *****************/
    for(unsigned int i = 0; i < get_stream_input_count(); i++)
    {
        avdecc_lib::stream_input_descriptor *stream_input_desc_ref = configuration->get_stream_input_desc_by_index(i);
        if(stream_input_desc_ref)
        {
            struct stream_configuration_details input_stream_details;
            
            avdecc_lib::stream_input_descriptor_response *stream_input_resp_ref = stream_input_desc_ref->get_stream_input_response();
            const uint8_t * object_name = stream_input_resp_ref->object_name();
            const uint8_t * stream_input_name;
            if(object_name[0] == '\0')
            {
                size_t string_desc_index;
                size_t string_index;
                int ret = configuration->get_strings_desc_string_by_reference(stream_input_resp_ref->localized_description(),
                                                                              string_desc_index, string_index);
                if(ret == 0)
                {
                    avdecc_lib::strings_descriptor *strings_desc = configuration->get_strings_desc_by_index(string_desc_index);
                    avdecc_lib::strings_descriptor_response *strings_resp_ref = strings_desc->get_strings_response();
                    
                    stream_input_name = strings_resp_ref->get_string_by_index(string_index);
                    input_stream_details.stream_name = stream_input_name;
                    delete strings_resp_ref;
                }
            }
            else
            {
                stream_input_name = object_name;
                input_stream_details.stream_name = stream_input_name;
            }
            
            const char * current_format = stream_input_resp_ref->current_format();
            uint64_t value = avdecc_lib::utility::ieee1722_format_name_to_value(current_format);
            value = value << 44;
            value = value >> 52;
            
            if(value == 1) //channel count 1
            {
                input_stream_details.channel_count = 1;
            }
            else if(value == 2) //channel count 2
            {
                input_stream_details.channel_count = 2;
            }
            else //channel count 2
            {
                input_stream_details.channel_count = 8;
            }
            
            input_stream_details.clk_sync_src_flag = stream_input_resp_ref->stream_flags_clock_sync_source();
            avdecc_input_stream_config.push_back(input_stream_details);
            
            delete stream_input_resp_ref;
        }
        else
        {
            std::cout << "get stream_input error" << std::endl;
            return 1;
        }
    }
    
    /****************** Stream Output Info *****************/
    for(unsigned int i = 0; i < get_stream_output_count(); i++)
    {
        avdecc_lib::stream_output_descriptor *stream_output_desc_ref = configuration->get_stream_output_desc_by_index(i);
        if(stream_output_desc_ref)
        {
            struct stream_configuration_details output_stream_details;
            
            avdecc_lib::stream_output_descriptor_response *stream_output_resp_ref = stream_output_desc_ref->get_stream_output_response();
            const uint8_t * object_name = stream_output_resp_ref->object_name();
            const uint8_t * stream_output_name;
            if(object_name[0] == '\0')
            {
                size_t string_desc_index;
                size_t string_index;
                int ret = configuration->get_strings_desc_string_by_reference(stream_output_resp_ref->localized_description(),
                                                                              string_desc_index, string_index);
                if(ret == 0)
                {
                    avdecc_lib::strings_descriptor *strings_desc = configuration->get_strings_desc_by_index(string_desc_index);
                    avdecc_lib::strings_descriptor_response *strings_resp_ref = strings_desc->get_strings_response();
                    
                    stream_output_name = strings_resp_ref->get_string_by_index(string_index);
                    output_stream_details.stream_name = stream_output_name;
                    delete strings_resp_ref;
                }
            }
            else
            {
                stream_output_name = object_name;
                output_stream_details.stream_name = stream_output_name;
            }
            
            const char * current_format = stream_output_resp_ref->current_format();
            uint64_t value = avdecc_lib::utility::ieee1722_format_name_to_value(current_format);
            value = value << 44;
            value = value >> 52;
            
            if(value == 1) //channel count 1
            {
                output_stream_details.channel_count = 1;
            }
            else if(value == 2) //channel count 2
            {
                output_stream_details.channel_count = 2;
            }
            else //channel count 8
            {
                output_stream_details.channel_count = 8;
            }
            
            output_stream_details.clk_sync_src_flag = stream_output_resp_ref->stream_flags_clock_sync_source();
            
            avdecc_output_stream_config.push_back(output_stream_details);
            delete stream_output_resp_ref;
        }
        else
        {
            std::cout << "get stream_output error" << std::endl;
            return 1;
        }
    }
    
    size_t cluster_count = configuration->audio_cluster_desc_count();
    
    size_t input_cluster_count = 0;
    size_t output_cluster_count = 0;
    
    for(size_t i = 0; i < cluster_count; i++)
    {
        avdecc_lib::audio_cluster_descriptor *audio_cluster_desc = configuration->get_audio_cluster_desc_by_index(i);
        avdecc_lib::audio_cluster_descriptor_response *audio_cluster_resp_ref = audio_cluster_desc->get_audio_cluster_response();
        if(audio_cluster_resp_ref->signal_type() == NO_STRING)
        {
            input_cluster_count++;
        }
        else
        {
            output_cluster_count++;
        }
    }
    
    set_input_output_cluster_counts(input_cluster_count, output_cluster_count);
    return 0;
}

int stream_configuration::GetAudioMappings()
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);
    
    intptr_t cmd_notification_id = get_next_notification_id();
    m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref = configuration->get_stream_port_input_desc_by_index(0); //index 0 returns all maps
    if(stream_port_input_desc_ref)
    {
        stream_port_input_desc_ref->send_get_audio_map_cmd((void *)cmd_notification_id, 0);
        int status = m_sys->get_last_resp_status();
        
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_port_input_get_audio_map_response *stream_port_input_resp_ref = stream_port_input_desc_ref->
            get_stream_port_input_audio_map_response();
            uint16_t nmappings = stream_port_input_resp_ref->number_of_mappings();
            
            for (int i = 0; i < (int)nmappings; i++)
            {
                struct audio_mapping m_map;
                struct avdecc_lib::stream_port_input_audio_mapping input_map;
                
                int ret = stream_port_input_resp_ref->mapping(i, input_map);
                
                if (ret == 0)
                {
                    m_map.stream_channel = input_map.stream_channel;
                    m_map.stream_index = input_map.stream_index;
                    m_map.cluster_offset = input_map.cluster_offset;
                    m_map.cluster_channel = input_map.cluster_channel;
                }
                avdecc_stream_port_input_audio_mappings.push_back(m_map); //add mapping to stream_config class
            }
            delete stream_port_input_resp_ref;
        }
        else
        {
            std::cout << "cmd_get_audio_map error" << std::endl;
        }
    }
    else
    {
        std::cout << "get stream port input error" << std::endl;
    }
    
    cmd_notification_id = get_next_notification_id();
    m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref = configuration->get_stream_port_output_desc_by_index(0); //index 0 returns all maps
    if(stream_port_output_desc_ref)
    {
        stream_port_output_desc_ref->send_get_audio_map_cmd((void *)cmd_notification_id, 0);
        int status = m_sys->get_last_resp_status();
        
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_port_output_get_audio_map_response *stream_port_output_resp_ref = stream_port_output_desc_ref->
            get_stream_port_output_audio_map_response();
            uint16_t nmappings = stream_port_output_resp_ref->number_of_mappings();
            
            for (int i = 0; i < (int)nmappings; i++)
            {
                struct audio_mapping m_map;
                struct avdecc_lib::stream_port_output_audio_mapping output_map;
                
                int ret = stream_port_output_resp_ref->mapping(i, output_map);
                
                if (ret == 0)
                {
                    m_map.stream_channel = output_map.stream_channel;
                    m_map.stream_index = output_map.stream_index;
                    m_map.cluster_offset = output_map.cluster_offset;
                    m_map.cluster_channel = output_map.cluster_channel;
                }
                avdecc_stream_port_output_audio_mappings.push_back(m_map); //add mapping to stream_config class
            }
            delete stream_port_output_resp_ref;
        }
        else
        {
            std::cout << "get audio map error" << std::endl;
        }
    }
    else
    {
        std::cout << "get stream port output error" << std::endl;
    }
    return 0;
}

int stream_configuration::SetStreamInfo()
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);
    
    avdecc_lib::audio_unit_descriptor *audio_desc = configuration->get_audio_unit_desc_by_index(0);
    avdecc_lib::audio_unit_descriptor_response *audio_unit_resp_ref = audio_desc->get_audio_unit_response();
    uint32_t current_sample_rate = audio_unit_resp_ref->current_sampling_rate();
    
    for(unsigned int i = 0; i < m_stream_input_count; i++)
    {
        struct stream_configuration_details avdecc_stream_input_details;
        struct stream_configuration_details dialog_stream_input_details;
        
        get_avdecc_stream_input_details_by_index(i, avdecc_stream_input_details);
        get_dialog_stream_input_details_by_index(i, dialog_stream_input_details);
        
        if(dialog_stream_input_details.channel_count != avdecc_stream_input_details.channel_count)
        {
            uint64_t stream_index = channel_count_and_sample_rate_to_stream_format(dialog_stream_input_details.channel_count,
                                                                                   current_sample_rate);
            if(stream_index != -1)
            {
                cmd_set_stream_format("STREAM_INPUT", i, stream_index);
            }
            else
            {
                std::cout << "Invalid Stream Format" << std::endl;
            }
        }
        if(dialog_stream_input_details.stream_name.IsSameAs(avdecc_stream_input_details.stream_name))
        {
            std::cout << "Stream Input Name unchanged" << std::endl;
        }
        else
        {
            cmd_set_name("STREAM_INPUT", i, std::string(dialog_stream_input_details.stream_name));
        }
    }
    
    for(unsigned int i = 0; i < m_stream_output_count; i++)
    {
        struct stream_configuration_details avdecc_stream_output_details;
        struct stream_configuration_details dialog_stream_output_details;
        
        get_avdecc_stream_output_details_by_index(i, avdecc_stream_output_details);
        get_dialog_stream_output_details_by_index(i, dialog_stream_output_details);
        
        if(dialog_stream_output_details.channel_count != avdecc_stream_output_details.channel_count)
        {
            uint64_t stream_index = channel_count_and_sample_rate_to_stream_format(dialog_stream_output_details.channel_count,
                                                                                   current_sample_rate);
            if(stream_index != -1)
            {
                cmd_set_stream_format("STREAM_OUTPUT", i, stream_index);
            }
            else
            {
                std::cout << "Invalid Stream Format" << std::endl;
            }
        }
        
        if(dialog_stream_output_details.stream_name.IsSameAs(avdecc_stream_output_details.stream_name))
        {
            std::cout << "Stream Output Name unchanged" << std::endl;
        }
        else
        {
            cmd_set_name("STREAM_OUTPUT", i, std::string(dialog_stream_output_details.stream_name.mb_str()));
        }
    }
    return 0;
}

int stream_configuration::SetAudioMappings()
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(m_end_station, &entity, &configuration);
    std::vector <avdecc_lib::audio_map_mapping> replaced_input_mappings;
    std::vector <avdecc_lib::audio_map_mapping> replaced_output_mappings;
    
    avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref = configuration->get_stream_port_input_desc_by_index(0); //index 0 returns all maps
    avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref = configuration->get_stream_port_output_desc_by_index(0); //index 0 returns all maps
    if(stream_port_input_desc_ref && stream_port_output_desc_ref)
    {
        //Input mappings for removal
        for(size_t it = 0; it < this->avdecc_stream_port_input_audio_mappings.size(); it++)
        {
            bool avdecc_input_mapping_found = false;
            bool avdecc_input_mapping_replaced = false;
            struct audio_mapping avdecc_mapping;
            
            avdecc_mapping = this->avdecc_stream_port_input_audio_mappings.at(it);
            
            for(size_t j = 0; j < this->dialog_stream_port_input_audio_mappings.size(); j++)
            {
                struct audio_mapping dialog_mapping;
                
                dialog_mapping = this->dialog_stream_port_input_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    avdecc_input_mapping_found = true;
                }
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset != avdecc_mapping.cluster_offset)
                {
                    avdecc_input_mapping_replaced = true;
                }
            }
            
            if(!avdecc_input_mapping_found || avdecc_input_mapping_replaced)
            {
                struct avdecc_lib::audio_map_mapping input_audio_map;
                
                input_audio_map.stream_channel = avdecc_mapping.stream_channel;
                input_audio_map.stream_index = avdecc_mapping.stream_index;
                input_audio_map.cluster_offset = avdecc_mapping.cluster_offset;
                input_audio_map.cluster_channel = avdecc_mapping.cluster_channel;
                
                stream_port_input_desc_ref->store_pending_map(input_audio_map);
            }
        }
        
        //Output mappings for removal
        for(size_t it = 0; it < this->avdecc_stream_port_output_audio_mappings.size(); it++)
        {
            bool avdecc_output_mapping_found = false;
            bool avdecc_output_mapping_replaced = false;
            struct audio_mapping avdecc_mapping;
            
            avdecc_mapping = this->avdecc_stream_port_output_audio_mappings.at(it);
            
            for(size_t j = 0; j < this->dialog_stream_port_output_audio_mappings.size(); j++)
            {
                struct audio_mapping dialog_mapping;
                
                dialog_mapping = this->dialog_stream_port_output_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    avdecc_output_mapping_found = true;
                }
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset != avdecc_mapping.cluster_offset)
                {
                    avdecc_output_mapping_replaced = true;
                }
            }
            
            if(!avdecc_output_mapping_found || avdecc_output_mapping_replaced)
            {
                struct avdecc_lib::audio_map_mapping output_audio_map;
                
                output_audio_map.stream_channel = avdecc_mapping.stream_channel;
                output_audio_map.stream_index = avdecc_mapping.stream_index;
                output_audio_map.cluster_offset = avdecc_mapping.cluster_offset;
                output_audio_map.cluster_channel = avdecc_mapping.cluster_channel;
                
                stream_port_output_desc_ref->store_pending_map(output_audio_map);
            }
        }

        if(stream_port_input_desc_ref->get_number_of_pending_maps())
        {
            remove_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_INPUT);
        }
        
        if(stream_port_output_desc_ref->get_number_of_pending_maps())
        {
            remove_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT);
        }
 
        //Input Mappings for adding
        for(size_t it = 0; it < this->dialog_stream_port_input_audio_mappings.size(); it++)
        {
            bool dialog_input_mapping_found = false;

            struct audio_mapping dialog_mapping;
            
            dialog_mapping = this->dialog_stream_port_input_audio_mappings.at(it);
            
            for(size_t j = 0; j < this->avdecc_stream_port_input_audio_mappings.size(); j++)
            {
                struct audio_mapping avdecc_mapping;
                
                avdecc_mapping = this->avdecc_stream_port_input_audio_mappings.at(j);
                
                if((dialog_mapping.stream_index == avdecc_mapping.stream_index) &&
                   (dialog_mapping.stream_channel == avdecc_mapping.stream_channel) &&
                   (dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset) &&
                   (dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel))
                {
                    dialog_input_mapping_found = true;
                }
            }
            
            if(dialog_input_mapping_found == false)
            {
                struct avdecc_lib::audio_map_mapping added_input_audio_map;
                
                added_input_audio_map.stream_index = dialog_mapping.stream_index;
                added_input_audio_map.stream_channel = dialog_mapping.stream_channel;
                added_input_audio_map.cluster_offset = dialog_mapping.cluster_offset;
                added_input_audio_map.cluster_channel = dialog_mapping.cluster_channel;

                if(added_input_audio_map.cluster_offset >= 0 && added_input_audio_map.cluster_offset <
                   m_stream_input_cluster_count)
                {
                    stream_port_input_desc_ref->store_pending_map(added_input_audio_map);
                }
                else
                {
                    std::cout << "not sending bad input mapping" << std::endl;
                }
            }
        }
        
        //Output mappings for adding
        for(size_t it = 0; it < this->dialog_stream_port_output_audio_mappings.size(); it++)
        {
            bool dialog_output_mapping_found = false;
            struct audio_mapping dialog_mapping;
            
            dialog_mapping = this->dialog_stream_port_output_audio_mappings.at(it);
            
            for(size_t j = 0; j < this->avdecc_stream_port_output_audio_mappings.size(); j++)
            {
                struct audio_mapping avdecc_mapping;
                
                avdecc_mapping = this->avdecc_stream_port_output_audio_mappings.at(j);
                
                if(dialog_mapping.stream_index == avdecc_mapping.stream_index &&
                   dialog_mapping.stream_channel == avdecc_mapping.stream_channel &&
                   dialog_mapping.cluster_offset == avdecc_mapping.cluster_offset &&
                   dialog_mapping.cluster_channel == avdecc_mapping.cluster_channel)
                {
                    dialog_output_mapping_found = true;
                }
            }
            
            if(dialog_output_mapping_found == false)
            {
                struct avdecc_lib::audio_map_mapping added_output_audio_map;
                
                added_output_audio_map.stream_index = dialog_mapping.stream_index;
                added_output_audio_map.stream_channel = dialog_mapping.stream_channel;
                added_output_audio_map.cluster_offset = dialog_mapping.cluster_offset;
                added_output_audio_map.cluster_channel = dialog_mapping.cluster_channel;
                
                if(added_output_audio_map.cluster_offset >= m_stream_input_cluster_count && added_output_audio_map.cluster_offset <
                   m_stream_input_cluster_count + m_stream_output_cluster_count)
                {
                    stream_port_output_desc_ref->store_pending_map(added_output_audio_map);
                }
                else
                {
                    std::cout << "not sending out of range output mapping" << std::endl;
                }
            }
        }
        
        if(stream_port_input_desc_ref->get_number_of_pending_maps())
        {
            add_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_INPUT);
        }
        
        if(stream_port_output_desc_ref->get_number_of_pending_maps())
        {
            add_audio_mappings(avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT);
        }
    }

    else
    {
        std::cout << "get stream port error" << std::endl;
        return 1;
    }
    return 0;

}

uint64_t stream_configuration::channel_count_and_sample_rate_to_stream_format(unsigned int channel_count, uint32_t sampling_rate)
{
    sampling_rate /= 1000; //extract first 2 digits
    
    std::ostringstream os;
    os << "IEC..." << sampling_rate << "KHZ_" << channel_count << "CH";
    std::string s = os.str();
    
    return avdecc_lib::utility::ieee1722_format_name_to_value(s.c_str());
}

int stream_configuration::cmd_set_name(std::string desc_name, uint16_t desc_index, std::string new_name)
{
    int status = avdecc_lib::AEM_STATUS_NOT_IMPLEMENTED;
    bool is_entity = false;
    
    uint16_t desc_type = avdecc_lib::utility::aem_desc_name_to_value(desc_name.c_str());
    
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    avdecc_lib::descriptor_base *desc_base = NULL;
    
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity,
                                                      &configuration))
        return 0;
    
    if(desc_type == avdecc_lib::AEM_DESC_ENTITY)
    {
        desc_base = entity;
        is_entity = true;
    }
    else if(desc_type == avdecc_lib::AEM_DESC_CONFIGURATION)
    {
        desc_base = dynamic_cast<avdecc_lib::descriptor_base *>(configuration);
    }
    else
    {
        desc_base = configuration->lookup_desc(desc_type, desc_index);
    }
    
    if(!desc_base)
    {
        std::cout << "cmd_set_name cannot lookup descriptor" << std::endl;
        return 0;
    }
    
    uint16_t name_index = 0;
    struct avdecc_lib::avdecc_lib_name_string64 new_name64 = {{0}};
    strncpy((char *)new_name64.value, new_name.c_str(),
            sizeof(new_name64.value));
    
    intptr_t cmd_notification_id = get_next_notification_id();
    m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
    desc_base->send_set_name_cmd((void *)cmd_notification_id, name_index, 0,
                                 + &new_name64);
    status = m_sys->get_last_resp_status();
    
    if(status == avdecc_lib::AEM_STATUS_SUCCESS)
    {
        cmd_display_desc_name(desc_base, name_index, is_entity);
    }
    else
    {
        std::cout << "cmd_set_name failed with AEM status: " <<
        avdecc_lib::utility::aem_cmd_status_value_to_name(status) <<
        std::endl;
    }
    
    return 0;
}

int stream_configuration::cmd_display_desc_name(avdecc_lib::descriptor_base *desc, uint16_t name_index, bool is_entity)
{
    uint8_t * name;
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity,
                                                      &configuration))
        return 0;
    
    if(is_entity == true)
    {
        avdecc_lib::entity_descriptor_response *entity_resp_ref = entity->get_entity_response();
        if(name_index == 0)
        {
            name = entity_resp_ref->entity_name();
        }
        else
        {
            name = entity_resp_ref->group_name();
        }
        delete entity_resp_ref;
    }
    else
    {
        avdecc_lib::descriptor_response_base *desc_resp_base = desc->get_descriptor_response();
        name = desc_resp_base->object_name();
        delete desc_resp_base;
    }
    
    if(!name)
    {
        std::cout << "cmd_set_name() failed" << std::endl;
    }
    else
    {
        std::cout << "Descriptor " <<
        avdecc_lib::utility::aem_desc_value_to_name(desc->descriptor_type()) <<
        "." << desc->descriptor_index() <<
        " name at index " << name_index << ": " <<
        std::dec << name << std::endl;
    }
    
    return 0;
}

int stream_configuration::add_audio_mappings(uint16_t desc_type)
{
    uint16_t desc_index = 0; //1 stream_port?
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity, &configuration))
        return 0;
    
    if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_INPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref= configuration->get_stream_port_input_desc_by_index(desc_index);
        stream_port_input_desc_ref->send_add_audio_mappings_cmd((void *)cmd_notification_id);
        m_sys->get_last_resp_status();
    }
    else if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref= configuration->get_stream_port_output_desc_by_index(desc_index);
        stream_port_output_desc_ref->send_add_audio_mappings_cmd((void *)cmd_notification_id);
        m_sys->get_last_resp_status();
    }
    else
    {
        std::cout << "Invalid Descriptor" << std::endl;
    }
    
    return 0;
}

int stream_configuration::remove_audio_mappings(uint16_t desc_type)
{
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity, &configuration))
        return 0;
    
    if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_INPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_input_descriptor *stream_port_input_desc_ref= configuration->get_stream_port_input_desc_by_index(0);
        stream_port_input_desc_ref->send_remove_audio_mappings_cmd((void *)cmd_notification_id);
        m_sys->get_last_resp_status();
    }
    else if(desc_type == avdecc_lib::AEM_DESC_STREAM_PORT_OUTPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_port_output_descriptor *stream_port_output_desc_ref= configuration->get_stream_port_output_desc_by_index(0);
        stream_port_output_desc_ref->send_remove_audio_mappings_cmd((void *)cmd_notification_id);
        m_sys->get_last_resp_status();
    }
    else
    {
        std::cout << "Invalid Descriptor" << std::endl;
    }
    
    return 0;
}

int stream_configuration::cmd_set_stream_format(wxString desc_name, uint16_t desc_index, uint64_t stream_format_value)
{
    std::string stream_format;
    
    uint16_t desc_type_value = avdecc_lib::utility::aem_desc_name_to_value(desc_name.c_str());
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    if (get_current_end_station_entity_and_descriptor(&m_end_station, &entity, &configuration))
        return 0;
    
    if(desc_type_value == avdecc_lib::AEM_DESC_STREAM_INPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_input_descriptor *stream_input_desc_ref = configuration->get_stream_input_desc_by_index(desc_index);
        stream_input_desc_ref->send_set_stream_format_cmd((void *)cmd_notification_id, stream_format_value);
        int status = m_sys->get_last_resp_status();
        
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_input_descriptor_response *stream_input_resp_ref = stream_input_desc_ref->get_stream_input_response();
            stream_format = stream_input_resp_ref->current_format();
            if(stream_format == "UNKNOWN")
            {
                std::cout << "Stream format: 0x" << std::hex << stream_input_resp_ref->current_format() << std::endl;
            }
            else
            {
                std::cout << "Stream format: " << stream_format << std::endl;
            }
            delete stream_input_resp_ref;
        }
    }
    else if(desc_type_value == avdecc_lib::AEM_DESC_STREAM_OUTPUT)
    {
        intptr_t cmd_notification_id = get_next_notification_id();
        m_sys->set_wait_for_next_cmd((void *)cmd_notification_id);
        avdecc_lib::stream_output_descriptor *stream_output_desc_ref = configuration->get_stream_output_desc_by_index(desc_index);
        stream_output_desc_ref->send_set_stream_format_cmd((void *)cmd_notification_id, stream_format_value);
        int status = m_sys->get_last_resp_status();
        
        if(status == avdecc_lib::AEM_STATUS_SUCCESS)
        {
            avdecc_lib::stream_output_descriptor_response *stream_output_resp_ref = stream_output_desc_ref->get_stream_output_response();
            stream_format = stream_output_resp_ref->current_format();
            if(stream_format == "UNKNOWN")
            {
                std::cout << "Stream format: 0x" << std::hex << stream_output_resp_ref->current_format() << std::endl;
            }
            else
            {
                std::cout << "Stream format: " << stream_format << std::endl;
            }
            delete stream_output_resp_ref;
        }
    }
    else
    {
        std::cout << "cmd_set_stream_format error" << std::endl;
    }
    
    return 0;
}

int stream_configuration::get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
                                                                 avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration)
{
    *entity = NULL;
    *configuration = NULL;
    
    uint16_t current_entity = end_station->get_current_entity_index();
    if (current_entity >= end_station->entity_desc_count())
    {
        std::cout << "Current entity not available" << std::endl;
        return 1;
    }
    
    *entity = end_station->get_entity_desc_by_index(current_entity);
    
    uint16_t current_config = end_station->get_current_config_index();
    if (current_config >= (*entity)->config_desc_count())
    {
        std::cout << "Current configuration not available" << std::endl;
        return 1;
    }
    
    *configuration = (*entity)->get_config_desc_by_index(current_config);
    
    return 0;
}

uint32_t stream_configuration::get_next_notification_id()
{
    return (uint32_t)notification_id++;
}

int stream_configuration::get_current_end_station_entity_and_descriptor(avdecc_lib::end_station **end_station,
                                                                             avdecc_lib::entity_descriptor **entity, avdecc_lib::configuration_descriptor **configuration)
{
    if (m_end_station == NULL)
        return 1;
    
    if (get_current_entity_and_descriptor(*end_station, entity, configuration))
    {
        std::cout << "Current End Station not fully enumerated" << std::endl;
        return 1;
    }
    return 0;
}

