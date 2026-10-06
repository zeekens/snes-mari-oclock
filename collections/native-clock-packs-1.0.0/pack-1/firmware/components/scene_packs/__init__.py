import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32
from esphome.const import CONF_ID
DEPENDENCIES = ['esp32', 'network']
ns=cg.esphome_ns.namespace('scene_packs')
ScenePacks=ns.class_('ScenePacks',cg.Component)
CONFIG_SCHEMA=cv.Schema({cv.GenerateID():cv.declare_id(ScenePacks)}).extend(cv.COMPONENT_SCHEMA)
async def to_code(config):
    var=cg.new_Pvariable(config[CONF_ID]);await cg.register_component(var,config)
    cg.add_define('SNES_REMOTE_SELECTED_ONLY')
    esp32.include_builtin_idf_component('esp_http_client')
    esp32.add_idf_sdkconfig_option('CONFIG_MBEDTLS_CERTIFICATE_BUNDLE',True)
