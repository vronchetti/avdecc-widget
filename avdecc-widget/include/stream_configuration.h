#include <iostream>
#include <vector>
#include <wx/string.h>

struct stream_configuration_details {
    wxString stream_name;
    unsigned int channel_count;
};

class stream_configuration
{
public:
    stream_configuration(unsigned int stream_input_count, unsigned int stream_output_count);
    virtual ~stream_configuration();

    std::vector<struct stream_configuration_details> input_stream_config;
    std::vector<struct stream_configuration_details> output_stream_config;
    
    unsigned int get_stream_input_count();
    unsigned int get_stream_output_count();
    int get_stream_input_name_by_index(unsigned int index, struct stream_configuration_details &stream_details);
    int get_stream_output_name_by_index(unsigned int index, struct stream_configuration_details &stream_details);

private:
    unsigned int m_stream_input_count;
    unsigned int m_stream_output_count;
};

