#pragma once

#include <WiFiNINA.h>

/** 
 * Circular buffer of logs, overwrittes old entries when reach the end
 * Send the log entries when http report to server
*/
struct web_log_t
{
    static const unsigned int MAX_ALLOC_BYTES = 8192;
    static const unsigned int MAX_MSG = 50;

    web_log_t()
    {
        messages_cursor_ = 0;
        for (int i=0; i<MAX_MSG; i++)
        {
            messages_[i] = String();
        }
    }

    inline void append(String s)
    {
        // Ignore if current message already full
        if (messages_[messages_cursor_].length() < (MAX_ALLOC_BYTES / MAX_MSG))
        {
            if (s.length() + messages_[messages_cursor_].length() > (MAX_ALLOC_BYTES / MAX_MSG))
            {
                // truncate incoming message when overflow current String
                s = s.substring(0, (MAX_ALLOC_BYTES / MAX_MSG) - messages_[messages_cursor_].length());
            }
            messages_[messages_cursor_] += s;
        }
    }

    inline void commit()
    {
        messages_[messages_cursor_].replace(" ", "+");
        messages_[messages_cursor_].replace("/", "%2F");
        messages_cursor_ += 1;
        if (messages_cursor_ >= MAX_MSG)
        {
            messages_cursor_ = 0;
        }
        // reset next entry to empty string
        messages_[messages_cursor_] = String();
    }

    inline void write_to_client(WiFiClient* client)
    {
        int cnt = 0;
        int cursor = messages_cursor_;
        while (cnt < MAX_MSG)
        {
            if (messages_[cursor].length() > 0)
            {
                client->print("&");
                client->print("log=");
                client->print(messages_[cursor].c_str());
            }
            cursor += 1;
            if (cursor >= MAX_MSG)
            {
                cursor = 0;
            }
            cnt += 1;
        }
    }

private:
    String messages_[MAX_MSG];
    int messages_cursor_;
};