//
//  end_station_configuration.cpp
//  avdecc-widget
//
//  Created by Victor on 3/2/15.
//  Copyright (c) 2015 Audioscience. All rights reserved.
//

#include "end_station_configuration.h"


end_station_configuration::end_station_configuration(wxString id_entity, wxString name_default, wxString mac_add,
                                                     wxString firmware_ver, uint32_t initial_sample_rate)
{
    entity_id = id_entity;
    default_name = name_default;
    mac = mac_add;
    fw_ver = firmware_ver;
    init_sample_rate = initial_sample_rate;
}

end_station_configuration::~end_station_configuration() {}

wxString end_station_configuration::get_entity_id()
{
    return entity_id;
}

wxString end_station_configuration::get_default_name()
{
    return default_name;
}

wxString end_station_configuration::get_mac()
{
    return mac;
}

wxString end_station_configuration::get_fw_ver()
{
    return fw_ver;
}

uint32_t end_station_configuration::get_init_sample_rate()
{
    return init_sample_rate;
}
