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
 * end_station_configuration.h
 *
 */

#include <iostream>
#include <wx/string.h>

class end_station_configuration
{
public:
    end_station_configuration(wxString entity_name, wxString id_entity, wxString name_default,
                              wxString mac_add, wxString firmware_ver, uint32_t initial_sample_rate);
    virtual ~end_station_configuration();
    
    wxString get_entity_name();
    wxString get_entity_id();
    wxString get_default_name();
    wxString get_mac();
    wxString get_fw_ver();
    uint32_t get_init_sample_rate();
    int set_sample_rate(uint32_t sampling_rate);

private:
    wxString name;
    wxString entity_id;
    wxString default_name;
    wxString mac;
    wxString fw_ver;
    uint32_t sample_rate;
};