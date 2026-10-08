import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.http_request import HttpRequestComponent
from esphome.const import CONF_ID, CONF_PASSWORD, CONF_USERNAME

DEPENDENCIES = ['http_request', 'esp32']
ns = cg.esphome_ns.namespace('firmware_transport')
FirmwareTransport = ns.class_('FirmwareTransport', HttpRequestComponent)
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(FirmwareTransport),
    cv.Required('http_request_id'): cv.use_id(HttpRequestComponent),
    cv.Required(CONF_USERNAME): cv.string,
    cv.Required(CONF_PASSWORD): cv.sensitive(cv.string),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    parent = await cg.get_variable(config['http_request_id'])
    cg.add(var.set_transport(parent))
    cg.add(var.set_credentials(config[CONF_USERNAME], config[CONF_PASSWORD]))
