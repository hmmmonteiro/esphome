#pragma once

#include "esphome.h"
#include "eQ3.h"
//#include "eQ3_message.h"
#include <BLEDevice.h>

namespace esphome {
namespace eqiva_lock {

//class Eqiva : public lock::Lock, public PollingComponent, public esphome::api::CustomAPIDevice {
class Eqiva : public lock::Lock, public PollingComponent {
    public:

        const std::string deviceName_ = "Eqiva ESPHome";

        void setup() override;
        void update() override;

        void set_is_paired(binary_sensor::BinarySensor *is_paired);
        void set_battery_critical(binary_sensor::BinarySensor *battery_critical);
        void set_card_key(const char* card_key);
        void set_key_turns(int key_turns);
        void set_mount_side(const char* mount_side);
        void set_unpair(bool unpair);

        void dump_config() override;

        bool retrieveCredentials();

    protected:
        void control(const lock::LockCall &call) override;
        //void update_status();
        void open_latch() override;

        binary_sensor::BinarySensor *is_paired_{nullptr};
        binary_sensor::BinarySensor *battery_critical_{nullptr};
        const char* card_key_{nullptr};
        int key_turns_;
        const char* mount_side_{nullptr};
        bool unpair_{false};
        eQ3* eqivaBle_;
        bool status_update_{false};
        bool open_latch_{false};
};

} //namespace eqiva_lock
} //namespace esphome
