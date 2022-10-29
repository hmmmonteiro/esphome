import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import lock, binary_sensor, switch
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_BATTERY,
)

AUTO_LOAD = ["binary_sensor", "switch"]

CONF_CARD_KEY = "card_key"
CONF_IS_PAIRED = "is_paired"
CONF_KEY_TURNS = "key_turns"
CONF_MOUNT_SIDE = "mount_side"
CONF_BATTERY_CRITICAL = "battery_critical"
CONF_UNPAIR = "unpair"

eqiva_lock_ns = cg.esphome_ns.namespace("eqiva_lock")
Eqiva = eqiva_lock_ns.class_("Eqiva", lock.Lock, switch.Switch, cg.Component)

CONFIG_SCHEMA = lock.LOCK_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(Eqiva),
        cv.Required(CONF_CARD_KEY): cv.string,
        cv.Required(CONF_IS_PAIRED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
        ),
        cv.Required(CONF_KEY_TURNS): cv.int_,
        cv.Required(CONF_MOUNT_SIDE): cv.string,
        cv.Optional(CONF_BATTERY_CRITICAL): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_BATTERY,
        ),
        cv.Optional(CONF_UNPAIR, default=False): cv.boolean,
    }
).extend(cv.polling_component_schema("500ms"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await lock.register_lock(var, config)

    if CONF_CARD_KEY in config:
        cg.add(var.set_card_key(config[CONF_CARD_KEY]))

    if CONF_IS_PAIRED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_IS_PAIRED])
        cg.add(var.set_is_paired(sens))

    if CONF_KEY_TURNS in config:
        cg.add(var.set_key_turns(config[CONF_KEY_TURNS]))

    if CONF_MOUNT_SIDE in config:
        cg.add(var.set_mount_side(config[CONF_MOUNT_SIDE]))

    if CONF_BATTERY_CRITICAL in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BATTERY_CRITICAL])
        cg.add(var.set_battery_critical(sens))

    if CONF_UNPAIR in config:
        cg.add(var.set_unpair(config[CONF_UNPAIR]))
