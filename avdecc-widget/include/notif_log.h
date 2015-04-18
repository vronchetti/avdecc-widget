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

#include <stdint.h>
#include <stdio.h>

struct notification_info
{
    uint32_t notification_type;
    uint64_t entity_id;
    uint16_t cmd_type;
    uint16_t desc_type;
    uint16_t desc_index;
    uint32_t cmd_status;
    void * notification_id;
};

struct log_info
{
    int32_t log_level;
    const char *log_msg;
};

std::vector<struct notification_info> pending_notification_msgs;
std::vector<struct log_info> pending_log_msgs;

extern "C" void notification_callback(void *user_obj, int32_t notification_type, uint64_t entity_id, uint16_t cmd_type,
                                      uint16_t desc_type, uint16_t desc_index, uint32_t cmd_status,
                                      void *notification_id)
{
    if(notification_type == avdecc_lib::COMMAND_TIMEOUT || notification_type == avdecc_lib::RESPONSE_RECEIVED)
    {
        const char *cmd_name;
        const char *desc_name;
        const char *cmd_status_name;
        
        if(cmd_type < avdecc_lib::CMD_LOOKUP)
        {
            cmd_name = avdecc_lib::utility::aem_cmd_value_to_name(cmd_type);
            desc_name = avdecc_lib::utility::aem_desc_value_to_name(desc_type);
            cmd_status_name = avdecc_lib::utility::aem_cmd_status_value_to_name(cmd_status);
        }
        else
        {
            cmd_name = avdecc_lib::utility::acmp_cmd_value_to_name(cmd_type - avdecc_lib::CMD_LOOKUP);
            desc_name = "NULL";
            cmd_status_name = avdecc_lib::utility::acmp_cmd_status_value_to_name(cmd_status);
        }
        
        printf("\n[NOTIFICATION] (%s, 0x%"  PRIx64 ", %s, %s, %d, %s, %p)\n",
               avdecc_lib::utility::notification_value_to_name(notification_type),
               entity_id,
               cmd_name,
               desc_name,
               desc_index,
               cmd_status_name,
               notification_id);
    }
    else
    {
        printf("\n[NOTIFICATION] (%s, 0x%"  PRIx64 ", %d, %d, %d, %d, %p)\n",
               avdecc_lib::utility::notification_value_to_name(notification_type),
               entity_id,
               cmd_type,
               desc_type,
               desc_index,
               cmd_status,
               notification_id);
    }
    
    notification_info m_notification_info;
    
    m_notification_info.notification_type = notification_type;
    m_notification_info.entity_id = entity_id;
    m_notification_info.cmd_type = cmd_type;
    m_notification_info.desc_type = desc_type;
    m_notification_info.desc_index = desc_index;
    m_notification_info.cmd_status = cmd_status;
    m_notification_info.notification_id = notification_id;
    
    pending_notification_msgs.push_back(m_notification_info);
}

extern "C" void log_callback(void *user_obj, int32_t log_level, const char *log_msg, int32_t time_stamp_ms)
{
    printf("\n[LOG] %s (%s)\n", avdecc_lib::utility::logging_level_value_to_name(log_level), log_msg);
    
    log_info m_log_info;
    
    m_log_info.log_level = log_level;
    m_log_info.log_msg = log_msg;
    
    pending_log_msgs.push_back(m_log_info);
}
