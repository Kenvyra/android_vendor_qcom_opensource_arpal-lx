/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <map>
#include <vector>
#include <memory>
#include <string>

#include "PalCommon.h"
#include "SoundTriggerXmlParser.h"
#include "kvh2xml.h"

const std::map<std::string, uint32_t> devicePPKeyLUT {
    {std::string{ "DEVICEPP_TX" }, DEVICEPP_TX},
};

const std::map<std::string, uint32_t> devicePPValueLUT {
    {std::string{ "DEVICEPP_TX_FLUENCE_FFNS" }, DEVICEPP_TX_FLUENCE_FFNS},
    {std::string{ "DEVICEPP_TX_FLUENCE_FFECNS" }, DEVICEPP_TX_FLUENCE_FFECNS},
    {std::string{ "DEVICEPP_TX_RAW_LPI" }, DEVICEPP_TX_RAW_LPI},
    {std::string{ "DEVICEPP_TX_RAW_NLPI" }, DEVICEPP_TX_RAW_NLPI},
};

CaptureProfile::CaptureProfile(const std::string name) :
    name_(name),
    device_id_(PAL_DEVICE_IN_MIN),
    sample_rate_(16000),
    channels_(1),
    bitwidth_(16),
    device_pp_kv_(std::make_pair(0, 0)),
    snd_name_("va-mic")
{

}

void CaptureProfile::HandleCharData(const char* data __unused) {
}

void CaptureProfile::HandleEndTag(const char* tag) {
    PAL_DBG(LOG_TAG, "Got end tag %s", tag);
    return;
}

void CaptureProfile::HandleStartTag(const char* tag, const char** attribs) {

    PAL_DBG(LOG_TAG, "Got start tag %s", tag);
    if (!strcmp(tag, "param")) {
        uint32_t i = 0;
        while (attribs[i]) {
            if (!strcmp(attribs[i], "device_id")) {
                auto itr = deviceIdLUT.find(attribs[++i]);
                if (itr == deviceIdLUT.end()) {
                    PAL_ERR(LOG_TAG, "could not find key %s in lookup table",
                        attribs[i]);
                } else {
                    device_id_ = itr->second;
                }
            } else if (!strcmp(attribs[i], "sample_rate")) {
                sample_rate_ = std::stoi(attribs[++i]);
            } else if (!strcmp(attribs[i], "bit_width")) {
                bitwidth_ = std::stoi(attribs[++i]);
            } else if (!strcmp(attribs[i], "channels")) {
                channels_ = std::stoi(attribs[++i]);
            } else if (!strcmp(attribs[i], "snd_name")) {
                snd_name_ = attribs[++i];
            } else {
                PAL_INFO(LOG_TAG, "Invalid attribute %s", attribs[i++]);
            }
            ++i; /* move to next attribute */
        }
    } else if (!strcmp(tag, "kvpair")) {
        uint32_t i = 0;
        uint32_t key = 0, value = 0;
        while (attribs[i]) {
            if (!strcmp(attribs[i], "key")) {
                auto keyItr = devicePPKeyLUT.find(attribs[++i]);
                if (keyItr == devicePPKeyLUT.end()) {
                    PAL_ERR(LOG_TAG, "could not find key %s in lookup table",
                        attribs[i]);
                } else {
                    key = keyItr->second;
                }
            } else if(!strcmp(attribs[i], "value")) {
                auto valItr = devicePPValueLUT.find(attribs[++i]);
                if (valItr == devicePPValueLUT.end()) {
                    PAL_ERR(LOG_TAG, "could not find value %s in lookup table",
                        attribs[i]);
                } else {
                    value = valItr->second;
                }

                device_pp_kv_ = std::make_pair(key, value);
            }
            ++i; /* move to next attribute */
        }
    } else {
        PAL_INFO(LOG_TAG, "Invalid tag %s", (char *)tag);
    }
}

/*
 * Priority compare result indicated by return value as below:
 * 1. CAPTURE_PROFILE_PRIORITY_HIGH
 *     current capture profile has higher priority than cap_prof
 * 2. CAPTURE_PROFILE_PRIORITY_LOW
 *     current capture profile has lower priority than cap_prof
 * 3. CAPTURE_PROFILE_PRIORITY_SAME
 *     current capture profile has same priority than cap_prof
 */
int32_t CaptureProfile::ComparePriority(std::shared_ptr<CaptureProfile> cap_prof) {
    int32_t priority_check = 0;

    if (!cap_prof) {
        priority_check = CAPTURE_PROFILE_PRIORITY_HIGH;
    } else {
        // only compare channels for priority for now
        if (channels_ < cap_prof->GetChannels()) {
            priority_check = CAPTURE_PROFILE_PRIORITY_LOW;
        } else if (channels_ > cap_prof->GetChannels()) {
            priority_check = CAPTURE_PROFILE_PRIORITY_HIGH;
        } else {
            priority_check = CAPTURE_PROFILE_PRIORITY_SAME;
        }
    }

    return priority_check;
}
