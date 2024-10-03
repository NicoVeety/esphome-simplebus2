import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins, automation
from esphome.const import CONF_ID, CONF_FILTER, CONF_IDLE, PLATFORM_ESP32
from esphome.core import coroutine_with_priority, TimePeriod

CODEOWNERS = ["@se-bastiaan"]
simplebus2_ns = cg.esphome_ns.namespace("simplebus2")
Simplebus2 = simplebus2_ns.class_("Simplebus2Component", cg.Component)

Simplebus2SendAction = simplebus2_ns.class_(
    "Simplebus2SendAction", automation.Action
)

CONF_SIMPLEBUS2_ID = "simplebus2"
CONF_COMMAND = "command"
CONF_ADDRESS = "address"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Simplebus2),
            cv.Optional(CONF_FILTER, default="1000us"): cv.All(
                cv.positive_time_period_microseconds,
                cv.Range(max=TimePeriod(microseconds=2500)),
            ),
            cv.Optional(CONF_IDLE, default="10ms"): cv.positive_time_period_microseconds,
        },
        cv.only_on([PLATFORM_ESP32]),
    )
    .extend(cv.COMPONENT_SCHEMA)
)

@coroutine_with_priority(1.0)
async def to_code(config):
    cg.add_global(simplebus2_ns.using)
    cg.add_library("Wire", None)
    
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    rx_pin = cg.gpio_pin_expression(2)
    cg.add(var.set_rx_pin(rx_pin))

    tx_pin = cg.gpio_pin_expression(3)
    cg.add(var.set_tx_pin(tx_pin))



SIMPLEBUS2_SEND_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(Simplebus2),
        cv.Required(CONF_COMMAND): cv.templatable(cv.hex_uint16_t),
        cv.Required(CONF_ADDRESS): cv.templatable(cv.hex_uint16_t),
    }
)


@automation.register_action(
    "simplebus2.send", Simplebus2SendAction, SIMPLEBUS2_SEND_SCHEMA
)
async def simplebus2_send_to_code(config, action_id, template_args, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_args, paren)
    template = await cg.templatable(config[CONF_COMMAND], args, cg.uint16)
    cg.add(var.set_command(template))
    template = await cg.templatable(config[CONF_ADDRESS], args, cg.uint16)
    cg.add(var.set_address(template))
    return var