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
 * end_station_configuration.cpp
 *
 */

#include "end_station_configuration.h"


end_station_configuration::end_station_configuration(wxString entity_name, wxString id_entity, wxString name_default,
                                                     wxString mac_add, wxString firmware_ver, uint32_t sampling_rate)
{
    name = entity_name;
    entity_id = id_entity;
    default_name = name_default;
    mac = mac_add;
    fw_ver = firmware_ver;
    sample_rate = sampling_rate;
}

end_station_configuration::~end_station_configuration() {}

wxString end_station_configuration::get_entity_id()
{
    return entity_id;
}

wxString end_station_configuration::get_entity_name()
{
    return name;
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

uint32_t end_station_configuration::get_sample_rate()
{
    return sample_rate;
}

int end_station_configuration::set_sample_rate(uint32_t sampling_rate)
{
    sample_rate = sampling_rate;
    return 0;
}
