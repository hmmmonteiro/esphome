#include "esphome.h"
#include "eQ3.h"
#include <BLEDevice.h>

namespace esphome {
namespace eqiva_lock {

static const char *const TAG = "eqiva_lock";

eQ3* eqivaBle_;
int status = -1;
bool do_open = false;
bool do_lock = false;
bool do_unlock = false;
bool do_status = false;

bool cmdTriggered=false;
unsigned long timeout=0;
bool statusUpdated=false;
bool waitForAnswer=false;
unsigned long starttime=0;


void Eqiva::set_is_paired(binary_sensor::BinarySensor *is_paired) { this->is_paired_ = is_paired; }
void Eqiva::set_battery_critical(binary_sensor::BinarySensor *battery_critical) { this->battery_critical_ = battery_critical; }
void Eqiva::set_card_key(const char* card_key) { this->card_key_ = card_key; }
void Eqiva::set_key_turns(int key_turns) { this->key_turns_ = key_turns; }
void Eqiva::set_mount_side(const char* mount_side) { this->mount_side_ = mount_side; }
void Eqiva::set_unpair(bool unpair) {this->unpair_ = unpair; }
void Eqiva::open_latch() { this->open_latch_ = true; unlock();}

/*******************************************************************************
 Setup
*******************************************************************************/
void Eqiva::setup() {


    ESP_LOGCONFIG(TAG, "Setting up Eqiva KEYBLE gateway...");
    ESP_LOGCONFIG(TAG, "%s", this->card_key_);

    BLEDevice::init("EqivaBLEgw");
    eqivaBle_ = new eQ3("00:1a:22:18:a6:5c", "58d139296c9f2fb8c12a047b2e9d7a36", 0);
    status = eqivaBle_->_LockStatus;

    if(this->retrieveCredentials()) {
        this->is_paired_->publish_initial_state(true);
    } else {
        this->is_paired_->publish_initial_state(false);
    }

    ESP_LOGI("main", "Lock Status is %d", status);
}

void Eqiva::update() {
    // Wifi reconnect
    if (statusUpdated) {
        statusUpdated = false;
        status = eqivaBle_->_LockStatus;
        String str_status="";
        char charBuffer[16];
        char charBuffer2[8];
        if(status == 1)
          str_status = "moving";
        else if(status == 2)
          str_status = "unlocked";
        else if(status == 3)
          str_status = "locked";
        else if(status == 4)
          str_status = "open";
        else if(status == 9)
          str_status = "timeout";
        else
          str_status = "unknown";
        String strBuffer =  String(str_status);
        strBuffer.toCharArray(charBuffer, 16);
        /*
        mqttClient.publish(MQTT_TOPIC_PUB, charBuffer);

        if(eqivaBle_->raw_data[0] == 0x72){  

        sprintf (charBuffer2, "%02X %02X %02X %02X %02X %02X %02X %02X", eqivaBle_->raw_data.c_str()[0]
        , eqivaBle_->raw_data.c_str()[1]
        , eqivaBle_->raw_data.c_str()[2]
        , eqivaBle_->raw_data.c_str()[3]
        , eqivaBle_->raw_data.c_str()[4]
        , eqivaBle_->raw_data.c_str()[5]
        , eqivaBle_->raw_data.c_str()[6]
        , eqivaBle_->raw_data.c_str()[7]);
        
        mqttClient.publish(MQTT_TOPIC_PUB2, charBuffer2);
        }
        */
        // if(status == 9)
        // {
        //   Serial.println("ESP Restarting...");
        //   mqttClient.loop();
        //   yield();
        //   delay(2000);
          
        //   //ESP.restart();
        // }
    }
    //}

    if (do_open || do_lock || do_unlock || do_status) {
        //SetWifi(false);
        //btStart();
        yield();
        waitForAnswer=true;
        eqivaBle_->_LockStatus = -1;
        starttime = millis();

        if (do_open) {
            Serial.println("Opening");
            //SetWifi(false);
            eqivaBle_->open();
            //SetWifi(true);
            do_open = false;
        }

        if (do_lock) {
            Serial.println("Locking");
            //SetWifi(false);
            eqivaBle_->lock();
            //SetWifi(true);
            do_lock = false;
        }

        if (do_unlock) {
            Serial.println("Unlocking");
            //SetWifi(false);
            eqivaBle_->unlock();
            //SetWifi(true);
            do_unlock = false;
        }

        if (do_status) {
            Serial.println("Status");
            //SetWifi(false);
            eqivaBle_->updateInfo();
            //SetWifi(true);
            do_status = false;
        }
    }

    if(waitForAnswer) {
      
        bool timeout=(millis() - starttime > LOCK_TIMEOUT *1000 +1000);
        bool finished=false;

        if ((eqivaBle_->_LockStatus != -1) || timeout) {
            switch (eqivaBle_->_LockStatus) {

                case 1:
                    finished=true;
                    Serial.println("Lockstatus 1 - timeout");
                    break;

                case -1:
                    eqivaBle_->_LockStatus = 9; //timeout
                    finished=true;
                    Serial.println("Lockstatus -1 - timeout");
                    break;

                default:
                    finished=true;
                    Serial.println("Lockstatus != 1");
            }

            if(finished) {
                Serial.println("finshed.");
          
                do {
                    eqivaBle_->bleClient->disconnect();
                    delay(100);
                }
                //while(eqivaBle_->state.connectionState != DISCONNECTED && !timeout);
                while(eqivaBle_->state.connectionState != DISCONNECTED);

                //btStop();
                delay(500);
                yield();
                Serial.println("# Lock status changed or timeout ...");
                // Serial.print("Data received: ");
         

          
                //SetWifi(true);
                waitForAnswer=false;
                statusUpdated=true;
            }
        }
    }
}

void Eqiva::control(const lock::LockCall &call) {
//    if (this->eqivaBle_->isPairedWithLock()) {
    if (true) {
        
        auto state = *call.get_state();

        uint8_t result;

        if (state == lock::LOCK_STATE_LOCKED) {
            eqivaBle_->lock();
            result = eqivaBle_->_LockStatus;
        } else if (state == lock::LOCK_STATE_UNLOCKED) {
            this->open_latch_ ? eqivaBle_->open() : eqivaBle_->unlock();
            this->open_latch_ = false;
            //eqivaBle_->open();
            result = eqivaBle_->_LockStatus;
        }
        else {
            ESP_LOGE(TAG, "lockAction unsupported state");
            return;
        }
        this->publish_state(state);
    }
    else {
        ESP_LOGE(TAG, "Lock/Unlock action called for unpaired Eqiva SmartLock");
    }
    ESP_LOGE(TAG, "%d", this->eqivaBle_->_LockStatus);
}

bool Eqiva::retrieveCredentials() {
  //TODO check on empty (invalid) credentials?
/*
  unsigned char buff[6];

  if ((preferences.getBytes(BLE_ADDRESS_STORE_NAME, buff, 6) > 0)
      && (preferences.getBytes(SECURITY_PINCODE_STORE_NAME, &pinCode, 2) > 0)
      && (preferences.getBytes(SECRET_KEY_STORE_NAME, secretKeyK, 32) > 0)
      && (preferences.getBytes(AUTH_ID_STORE_NAME, authorizationId, 4) > 0)
     ) {
    bleAddress = BLEAddress(buff);

    log_d("[%s] Credentials retrieved :", deviceName.c_str());
    printBuffer(secretKeyK, sizeof(secretKeyK), false, SECRET_KEY_STORE_NAME);
    log_d("bleAddress: %s", bleAddress.toString().c_str());
    printBuffer(authorizationId, sizeof(authorizationId), false, AUTH_ID_STORE_NAME);
    log_d("PinCode: %d", pinCode);

    //if (pinCode == 0) {
    //  log_w("Pincode is 000000");
    //}

  } else {
    return false;
  }*/
  return true;
}


void Eqiva::dump_config(){
    LOG_LOCK(TAG, "Eqiva Lock", this);
    LOG_BINARY_SENSOR(TAG, "Is Paired", this->is_paired_);
    LOG_BINARY_SENSOR(TAG, "Battery Critical", this->battery_critical_);
    ESP_LOGCONFIG(TAG, "Unpair request is %s", this->unpair_? "true":"false");
}

} //namespace eqiva_lock
} //namespace esphome
