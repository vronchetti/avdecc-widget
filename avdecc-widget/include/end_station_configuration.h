#include <iostream>
#include <wx/string.h>

class end_station_configuration
{
public:
    end_station_configuration(wxString id_entity, wxString name_default, wxString mac_add,
                              wxString firmware_ver, uint32_t initial_sample_rate);
    virtual ~end_station_configuration();
    
    wxString get_entity_id();
    wxString get_default_name();
    wxString get_mac();
    wxString get_fw_ver();
    uint32_t get_init_sample_rate();

private:
    wxString entity_id;
    wxString default_name;
    wxString mac;
    wxString fw_ver;
    uint32_t init_sample_rate;
};