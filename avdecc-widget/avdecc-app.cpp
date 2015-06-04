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

#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/utils.h>

#include "avdecc-app.h"
#include "notif_log.h"
#include "../sample.xpm"

class AVDECC_App : public wxApp
{
public:
    virtual bool OnInit() { (new AVDECC_Controller())->Show(); return true; }
};

wxBEGIN_EVENT_TABLE(AVDECC_Controller, wxFrame)
    EVT_TIMER(EndStationTimer, AVDECC_Controller::OnIncrementTimer)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, AVDECC_Controller::OnEndStationDClick)
    EVT_CHOICE(InterfaceSelect, AVDECC_Controller::OnInterfaceSelect)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(AVDECC_App)

AVDECC_Controller::AVDECC_Controller()
: wxFrame(NULL, wxID_ANY, wxT("AVDECC-LIB Controller widget"),
          wxDefaultPosition, wxSize(600,300))
{
    SetIcon(wxICON(sample));
    SetTimer();
    CreateLogging();
    CreateController();
    PrintAndSelectInterface();
    CreateEndStationListFormat();
}

AVDECC_Controller::~AVDECC_Controller()
{
    sys->process_close();
    sys->destroy();
    controller_obj->destroy();
    netif->destroy();
    avdecc_app_timer->Stop();
    delete wxLog::SetActiveTarget(NULL);
    //Destroy();
}

void AVDECC_Controller::SetTimer()
{
    avdecc_app_timer = new wxTimer(this, EndStationTimer);
    avdecc_app_timer->Start(TIMER_INCREMENT, wxTIMER_CONTINUOUS);
}

void AVDECC_Controller::CreateLogging()
{
    logs_notifs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(600, 300), wxTE_MULTILINE);
}

void AVDECC_Controller::CreateController()
{
    m_end_station_count = 0;
    notification_id = 1;
    netif = avdecc_lib::create_net_interface();
    controller_obj = avdecc_lib::create_controller(netif, notification_callback, log_callback, log_level);
    sys = avdecc_lib::create_system(avdecc_lib::system::LAYER2_MULTITHREADED_CALLBACK, netif, controller_obj);
}

void AVDECC_Controller::CreateEndStationList()
{
	wxMilliSleep(END_STATION_PROCESS_DELAY);

    for (unsigned int i = 0; i < controller_obj->get_end_station_count(); i++)
    {
        avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(i);
        
        if (end_station)
        {
            uint64_t end_station_entity_id = end_station->entity_id();
            avdecc_lib::entity_descriptor_response *ent_desc_resp = NULL;
            if (end_station->entity_desc_count())
            {
                uint16_t current_entity = end_station->get_current_entity_index();
                ent_desc_resp = end_station->get_entity_desc_by_index(current_entity)->get_entity_response();
            }
            const char *end_station_name = "";
            const char *fw_ver = "";
            if (ent_desc_resp)
            {
                end_station_name = (const char *)ent_desc_resp->entity_name();
                fw_ver = (const char *)ent_desc_resp->firmware_version();
            }
            uint64_t end_station_mac = end_station->mac();
            
            wxListItem item;
            item.SetId(i);
            details_list->InsertItem(item);
            details_list->SetItem(i, 0, end_station->get_connection_status());
            details_list->SetItem(i, 1, end_station_name);
            details_list->SetItem(i, 2, wxString::Format("0x%llx", end_station_entity_id));
            details_list->SetItem(i, 3, fw_ver);
            details_list->SetItem(i, 4, wxString::Format("%llx",end_station_mac));
            delete ent_desc_resp;
        }
        m_end_station_count++;
    }
}

void AVDECC_Controller::PrintAndSelectInterface()
{
    char *port = NULL;

    int interface_num = 1;
    wxArrayString str;
    
    for(uint32_t i = 1; i < netif->devs_count() + 1; i++)
    {
        char *dev_desc = netif->get_dev_desc_by_index(i - 1);
        if (!port)
        {
            str.Add(dev_desc);
        }
        else
        {
            if (strcmp(dev_desc, port) == 0)
            {
                interface_num = i;
                break;
            }
        }
    }

    interface_choice = new wxChoice(this, InterfaceSelect, wxDefaultPosition, wxSize(100,25), str);
}

void AVDECC_Controller::OnInterfaceSelect(wxCommandEvent &event)
{
    unsigned int index = interface_choice->GetSelection();
    
    netif->select_interface_by_num(index + 1);
    sys->process_start();

    CreateEndStationList();
}

void AVDECC_Controller::OnEndStationDClick(wxListEvent& event)
{
    avdecc_lib::end_station *end_station = controller_obj->get_end_station_by_index(event.GetIndex());
    avdecc_lib::entity_descriptor *entity;
    avdecc_lib::configuration_descriptor *configuration;
    get_current_entity_and_descriptor(end_station, &entity, &configuration);
    
    std::streambuf *sbOld = std::cout.rdbuf();
    std::cout.rdbuf(logs_notifs);

    config = new end_station_configuration(end_station, sys); //end station config class
    stream_config = new stream_configuration(end_station, sys); //stream config class

    details = new end_station_details(this, config, stream_config); //end station details dialog
    
    int retval = details->ShowModal();

    if (retval == wxID_CANCEL)
    {
        details->OnCancel();
        std::cout << "Cancel" << std::endl;
    }
    else if (retval == wxID_OK)
    {
        details->OnOK();
        std::cout << "Apply" << std::endl;
        details->Destroy();
    }
    else
    {
        //not supported
    }

    std::cout.rdbuf(sbOld);
	delete config;
	delete stream_config;
    delete details;
}

void AVDECC_Controller::OnIncrementTimer(wxTimerEvent& event)
{
    if(m_end_station_count < controller_obj->get_end_station_count())
    {
        details_list->DeleteAllItems();
        CreateEndStationList();
    }
    else
    {
        //no added end stations found
    }
    
    std::streambuf *sbOld = std::cout.rdbuf();
    std::cout.rdbuf(logs_notifs);
    
    for(size_t i = 0; i < pending_notification_msgs.size(); i++)
    {
        struct notification_info notification;
        
        notification = pending_notification_msgs.at(i);
		if (notification.notification_type == avdecc_lib::COMMAND_TIMEOUT || notification.notification_type == avdecc_lib::RESPONSE_RECEIVED)
		{
			const char *cmd_name;
			const char *desc_name;
			const char *cmd_status_name;

			if (notification.cmd_type < avdecc_lib::CMD_LOOKUP)
			{
				cmd_name = avdecc_lib::utility::aem_cmd_value_to_name(notification.cmd_type);
				desc_name = avdecc_lib::utility::aem_desc_value_to_name(notification.desc_type);
				cmd_status_name = avdecc_lib::utility::aem_cmd_status_value_to_name(notification.cmd_status);
			}
			else
			{
				cmd_name = avdecc_lib::utility::acmp_cmd_value_to_name(notification.cmd_type - avdecc_lib::CMD_LOOKUP);
				desc_name = "NULL";
				cmd_status_name = avdecc_lib::utility::acmp_cmd_status_value_to_name(notification.cmd_status);
			}

			std::cout << "[NOTIFICATION] " <<
				avdecc_lib::utility::notification_value_to_name(notification.notification_type) << " " <<
				wxString::Format("0x%llx", notification.entity_id) << " " <<
				cmd_name << " " <<
				desc_name << " " <<
				notification.desc_index << " " <<
				cmd_status_name << " " <<
				notification.notification_id << std::endl;
		}
		else
		{
			std::cout << "[NOTIFICATION] " <<
				avdecc_lib::utility::notification_value_to_name(notification.notification_type) << " " <<
				wxString::Format("0x%llx", notification.entity_id) << " " <<
				notification.cmd_type << " " <<
				notification.desc_type << " " <<
				notification.desc_index << " " <<
				notification.cmd_status << " " <<
				notification.notification_id << std::endl;
		}

    }
    
    pending_notification_msgs.clear();

    std::cout.rdbuf(sbOld);
}
 

void AVDECC_Controller::CreateEndStationListFormat()
{
    details_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition,
                                  wxSize(600,200), wxLC_REPORT);
    
    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("") );
    col0.SetWidth(25);
    details_list->InsertColumn(0, col0);
    
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Name") );
    col1.SetWidth(100);
    details_list->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetText( _("Entity ID") );
    col2.SetWidth(150);
    details_list->InsertColumn(2, col2);

    wxListItem col3;
    col3.SetId(3);
    col3.SetText( _("Firmware Version") );
    col3.SetWidth(150);
    details_list->InsertColumn(3, col3);

    wxListItem col4;
    col4.SetId(4);
    col4.SetText( _("MAC") );
    col4.SetWidth(150);
    details_list->InsertColumn(4, col4);
    
    wxBoxSizer * sizer1 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *sizer4 = new wxStaticBoxSizer(wxVERTICAL, this, "Messages");
    sizer4->Add(logs_notifs);
    wxStaticBoxSizer *sizer3 = new wxStaticBoxSizer(wxVERTICAL, this, "Select Interface");
    sizer3->Add(interface_choice);
    wxStaticBoxSizer *sizer2 = new wxStaticBoxSizer(wxVERTICAL, this, "End Station List");
    sizer2->Add(details_list, 1, wxGROW);
    sizer1->Add(sizer3);
    sizer1->Add(sizer2);
    sizer1->Add(sizer4);

    SetSizer(sizer1);
}

uint32_t AVDECC_Controller::get_next_notification_id()
{
    return (uint32_t)notification_id++;
}

int AVDECC_Controller::get_current_entity_and_descriptor(avdecc_lib::end_station *end_station,
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


